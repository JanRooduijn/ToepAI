#include "../../include/frontend/state-core.h"

#include <iostream>
using ImGui::operator+;
using ImGui::operator-;
using ImGui::operator*;

namespace {

using TColor = uint32_t;

}

//
// Rendering
//

void Rendering::init() {
    T = ImGui::GetTime();
    isAnimating = false;
    wSize = ImGui::GetContentRegionAvail();
}

//
// StateCore
//

StateCore::StateCore(Game& game) : game(game) {}

void StateCore::onWindowResize() {
}

void StateCore::updateDataDummy() {
    dataDummy = "foo bar";
}

//
// UI methods
//

void StateCore::init(float fontScale) {
    //const float T = ImGui::GetTime();

    this->fontScale = fontScale;

    printf("Initialized the application state\n");
    isInitialized = true;
}

void StateCore::render() {
    // shortcuts
    const auto & T     = rendering.T;
    const auto & wSize = rendering.wSize;

    if (rendering.isFirstFrame) {
        ImGui::SetStyle();
        rendering.isFirstFrame = false;
    }

    if (ImGui::IsMouseJustPressed(0)) game.unPause();

    // full-screen window with the scene
    {
        ImVec4 steelBlue = ImVec4(0.274f, 0.51f, 0.706f, 1.0f);
        ImGuiStyle& style = ImGui::GetStyle();
        style.Colors[ImGuiCol_WindowBg] = steelBlue;

        ImGui::SetNextWindowPos({ 0.0f, 0.0f });
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGui::BeginBorderless("background", NULL,
                               ImGuiWindowFlags_NoNav |
                               ImGuiWindowFlags_NoNavFocus |
                               ImGuiWindowFlags_NoBringToFrontOnFocus |
                               ImGuiWindowFlags_NoResize |
                               ImGuiWindowFlags_NoDecoration);

        // call after creating the main window
        rendering.init();

        cardSize = ImVec2(0.4f * 0.25f * wSize.x, 0.4f * 0.35f * wSize.x);
        cardSpace = cardSize.x / 10;
        handSize = ImVec2(4*cardSize.x + 3*cardSpace, cardSize.y);

        if (game.size() == 2) {
            drawPlayer(0, ImVec2(0, wSize.y), ImVec2(0, -1));
            drawPlayer(1, ImVec2(0, 0), ImVec2(0, 1));
        }
        if (game.size() == 4) {
            drawPlayer(0, ImVec2(0, wSize.y), ImVec2(0, -1));
            drawPlayer(1, ImVec2(0, 0), ImVec2(1,0));
            drawPlayer(2, ImVec2(0, 0), ImVec2(0, 1));
            drawPlayer(3, ImVec2(wSize.x, 0), ImVec2(-1, 0));
        }

        auto drawList = ImGui::GetWindowDrawList();
        {
            ImGui::FontSentry sentry(1, wSize.x / 2000 + wSize.y / 2000);
            float wagerPadding = 20.0f;
            ImVec2 wagerPos(wagerPadding, wSize.y - wagerPadding - ImGui::GetTextLineHeight());
            std::string wagerNo = std::to_string(game.getWager());
            std::string wagerHand = ICON_MDI_HAND_POINTING_UP;
            std::string wagerText = wagerHand + wagerNo;
            drawList->AddText(wagerPos, IM_COL32_BLACK, wagerText.c_str());
        }

        if (game.getWinner() != -1) {
            ImGui::FontSentry sentry(1, wSize.x / 1000 + wSize.y / 1000);

            auto drawList = ImGui::GetWindowDrawList();

            std::string winMsg = "Player " + std::to_string(game.getWinner()) + " wins!";
            ImVec2 textSize = ImGui::CalcTextSize(winMsg.c_str());
            ImVec2 textPos = 0.5f * (wSize - textSize);

            ImU32 lightGray = IM_COL32(200, 200, 200, 255);
            drawList->AddText(textPos, lightGray, winMsg.c_str());
        }
    }

    ImGui::End();
}

bool StateCore::updatePre() {
    //const float T = ImGui::GetTime();

#ifndef EMSCRIPTEN
    if (ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Escape))) {
        printf("Escape pressed - exiting\n");
        return false;
    }
#endif

    return true;
}

bool StateCore::updatePost() {
    return true;
}

void StateCore::deinitMain() {
}

void StateCore::drawCard(size_t playerIndex, size_t cardIndex, ImVec2 pos, const ImVec2& dir) {
    Player& player = game.getPlayer(playerIndex);
    const Card& card = player.getHand().getCard(cardIndex);
    if (!showFinishedCard && card.state() == Card::State::DONE) return;

    bool isAI = game.getPlayer(playerIndex).isAI();

    auto drawList = ImGui::GetWindowDrawList();
    const ImVec2& wSize = rendering.wSize;
    ImVec2 size;
    if (dir.x == 0) size = cardSize;
    else size = ImVec2(cardSize.y, cardSize.x);

    if (isAI && card.state() == Card::State::INIT) {
        drawCardBack(pos, dir);
        return;
    }

    if (card.state() == Card::State::IN_PLAY) {
        size = cardSize;
        auto cardsInPlay = game.getCardsInPlay();
        auto it = std::find(cardsInPlay.begin(), cardsInPlay.end(), &card);
        if (it == cardsInPlay.end()) throw std::runtime_error("Card in play not found");
        int index = std::distance(cardsInPlay.begin(), it);

        pos.x = (wSize.x - handSize.x) /2 + index * (cardSize.x + cardSpace);
        pos.y = (wSize.y - cardSize.y) / 2;
    }

    ImGui::SetCursorScreenPos(pos);
    ImGui::InvisibleButton("card_hover_area", size);
    bool isHovered = ImGui::IsItemHovered();

    int cardAlpha = 255;
    if (isHovered && !isAI && card.state() == Card::State::INIT && game.getState() == Game::State::PLAY && game.getCurrentPlayer() == playerIndex) cardAlpha = 180;
    if (card.state() == Card::State::DONE) cardAlpha = 30;

    if (!player.isAI() && ImGui::IsItemHovered() && ImGui::IsMouseJustPressed(0) && card.state() == Card::State::INIT && game.getCurrentPlayer() == playerIndex) {
        game.playCard(playerIndex, cardIndex);
    }

    ImU32 col = IM_COL32(255, 255, 255, cardAlpha);
    if (game.getState() == Game::State::TRICK_DONE && card.state() == Card::State::IN_PLAY && !card.won()) {
        cardAlpha = 100;
        col = IM_COL32(100, 100, 100, cardAlpha);
    }

    float rounding = 10.0f; // Rounding radius for card corners
    drawList->AddRectFilled(pos, pos + size, col, rounding);
    drawList->AddRect(pos, pos + size, IM_COL32(0, 0, 0, cardAlpha), rounding);

    if (card.isRed()) col = IM_COL32(255, 0, 0, cardAlpha);
    else col = IM_COL32(0, 0, 0, cardAlpha);

    // Draw value
    {
        ImGui::FontSentry sentry(1, wSize.x / 2000 + wSize.y / 2000);
        std::stringstream ss;
        switch (int value = card.value()) {
            case Card::JACK: ss << "J"; break;
            case Card::QUEEN: ss << "Q"; break;
            case Card::KING: ss << "K"; break;
            case Card::ACE: ss << "A"; break;
            default: ss << value; break;
        }
        std::string str = ss.str();
        ImVec2 textSize = ImGui::CalcTextSize(str.c_str());
        drawList->AddText(ImVec2(pos.x + 10, pos.y + 10), col, str.c_str());
        drawList->AddText(ImVec2(pos.x + size.x - textSize.x - 10, pos.y + size.y - textSize.y - 10), col, str.c_str());
    }

    // Draw suit
    {
        ImGui::FontSentry sentry(1, wSize.x / 1000 + wSize.y / 1000);
        std::stringstream ss;
        switch (int value = card.suit()) {
            case Card::CLUBS: ss << ICON_MDI_CARDS_CLUB; break;
            case Card::DIAMONDS: ss << ICON_MDI_CARDS_DIAMOND; break;
            case Card::HEARTS: ss << ICON_MDI_CARDS_HEART; break;
            case Card::SPADES: ss << ICON_MDI_CARDS_SPADE; break;
            default: ss << value; break;
        }
        std::string str = ss.str();
        ImVec2 textSize = ImGui::CalcTextSize(str.c_str());

        ImVec2 center;
        center.x = pos.x + (size.x * 0.5f) - (textSize.x * 0.5f);
        center.y = pos.y + (size.y * 0.5f) - (textSize.y * 0.5f);
        drawList->AddText(center, col, str.c_str());
    }
}

void StateCore::drawCardBack(const ImVec2& pos, const ImVec2& dir) {
    auto drawList = ImGui::GetWindowDrawList();
    const ImVec2& wSize = rendering.wSize;
    ImVec2 size;
    if (dir.x == 0) size = cardSize;
    else size = ImVec2(cardSize.y, cardSize.x);

    float rounding = 10.0f;
    drawList->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 204, 102, 255), rounding); // Dark blue background
    drawList->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(0, 0, 0, 255), rounding);

    float borderThickness = 3.0f;
    float distanceFromEdge = 10.0f;
    ImU32 innerBorderColor = IM_COL32(255, 255, 255, 255); // White color
    drawList->AddRect(ImVec2(pos.x + borderThickness + distanceFromEdge, pos.y + borderThickness + distanceFromEdge),
                      ImVec2(pos.x + size.x - borderThickness - distanceFromEdge, pos.y + size.y - borderThickness - distanceFromEdge),
                      innerBorderColor, rounding - borderThickness, 0, borderThickness);
}

void StateCore::drawHand(size_t playerIndex, ImVec2 pos, const ImVec2& dir) {
    Player& player = game.getPlayer(playerIndex);
    const Hand& hand = player.getHand();
    for (size_t i = 0; i < hand.size(); ++i) {
        drawCard(playerIndex, i, pos, dir);
        if (dir.x == 0) pos = ImVec2(pos.x + cardSize.x + cardSpace, pos.y);
        else pos = ImVec2(pos.x, pos.y + cardSize.x + cardSpace);
    }
}

void StateCore::drawToepButton(const ImVec2& pos) {
    // Define the upward-pointing arrow icon
    const char* buttonText = ICON_MDI_HAND_POINTING_UP;  // Use the upward-pointing arrow icon

    // Calculate the size of the text (icon)
    ImVec2 textSize = ImGui::CalcTextSize(buttonText);

    // Set the cursor position for the invisible button
    ImGui::SetCursorPos(pos);

    // Create an invisible button sized according to the icon's text size
    ImGui::InvisibleButton("up_arrow_button", textSize);

    // Check if the button is hovered or clicked
    bool isHovered = ImGui::IsItemHovered();
    bool isClicked = ImGui::IsItemActive();

    // Get the current window's draw list for rendering the icon manually
    ImDrawList* drawList = ImGui::GetWindowDrawList();

    // Define the color of the icon based on hover/click states
    ImU32 iconColor = IM_COL32(255, 255, 255, 255);  // Default: white
    if (isHovered)
        iconColor = IM_COL32(200, 200, 255, 255);  // Hovered: slightly tinted
    if (isClicked)
        iconColor = IM_COL32(150, 150, 255, 255);  // Active: stronger tint

    // Render the icon (upward pointing arrow)
    drawList->AddText(pos, iconColor, buttonText);

    // Handle the button click
    if (ImGui::IsItemClicked()) {
        // Perform the action when the button is clicked
        game.toep(0);
    }
}



void StateCore::drawPlayer(size_t playerIndex, const ImVec2& pos, const ImVec2& dir) {
    const ImVec2& wSize = rendering.wSize;

    ImGui::FontSentry sentry(1, wSize.x / 1000 + wSize.y / 1000);
    auto drawList = ImGui::GetWindowDrawList();

    // Draw score
    ImVec2 scoreDist = ImGui::CalcTextSize(tally(5, true).c_str());
    std::string score = tally(game.getPlayer(playerIndex).score(), dir.x == 0);
    if (game.getState() == Game::State::TOEP && game.isStartingToeper(playerIndex)) {
        std::string prefix = std::string(ICON_MDI_HAND_POINTING_UP) + std::string("+") + "1";
        if (dir.x != 0) prefix += "\n";
        score = score + prefix;
    }
    ImVec2 textSize = ImGui::CalcTextSize(score.c_str());
    ImVec2 textPos;
    if (dir.x == 0) textPos = ImVec2(0.5f * (wSize.x - textSize.x), pos.y + dir.y * 0.2f * scoreDist.y);
    else textPos = ImVec2(pos.x + dir.x * 0.2f * scoreDist.x, 0.5f * (wSize.y - textSize.y));
    if (dir.y < 0) {
        textPos.y -= textSize.y;
    }
    if (dir.x < 0) {
        textPos.x -= textSize.x;
    }
    ImU32 black = IM_COL32(0, 0, 0, 255);
    drawList->AddText(textPos, black, score.c_str());

    // Draw hand
    ImVec2 handPos;
    if (dir.x == 0) handPos = ImVec2(0.5f * (wSize.x - handSize.x), pos.y + dir.y * 1.5f * scoreDist.y);
    else handPos = ImVec2(pos.x + dir.x * 1.5f * scoreDist.x, 0.5f * (wSize.y - handSize.y));
    if (dir.y <= 0) {
        handPos.y -= handSize.y;
    }
    if (dir.x < 0) {
        handPos.x -= handSize.y;
    }

    drawHand(playerIndex, handPos, dir);

    // Draw toep button if Player is not an AI
    if (!game.getPlayer(playerIndex).isAI()) {
        drawToepButton(ImVec2(handPos.x, textPos.y));
    }
}

std::string StateCore::tally(size_t score, bool horizontal) {
    std::stringstream ss;
    for (size_t i = 0; i < score / 5; ++i) {
        ss << ICON_MDI_TALLY_MARK_5;
        if (!horizontal) ss << std::endl;
    }
    size_t rem = score % 5;
    switch(rem) {
        case 1: ss << ICON_MDI_TALLY_MARK_1; break;
        case 2: ss << ICON_MDI_TALLY_MARK_2; break;
        case 3: ss << ICON_MDI_TALLY_MARK_3; break;
        case 4: ss << ICON_MDI_TALLY_MARK_4; break;
    }
    return ss.str();
}


