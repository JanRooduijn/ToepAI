#pragma once
#include "common.h"
#include <imgui/imgui.h>
#include <string>
#include "../backend/game.h"
#include "../frontend/icons-font-awesome.h"
#include "../frontend/icons-material-design-icons.h"
#include <cmath>
#include <sstream>


// helper struct to manage the rendering state
struct Rendering {
    float T = 0.0f;
    bool isAnimating = false;
    ImVec2 wSize;

    int nUpdates = 60;
    bool isFirstFrame = true;

    // call this at the start of each frame to initialize helper variables
    void init();

    // when there is an active animation, call this method to disable any framerate throttling that may occur
    // the argument i is an interpolation factor of the animation
    //  - if it is within (0, 1) then the animation is in progress
    //  - if it is >= 1 then the animation has finished
    //  - if it is  < 0 then the animation will start soon
    //  - if it is == 0 then the animation is disabled
    //
    void animation(float i);
};

//
// the main application logic is implemented in this class
//
// ideally, this can be separated in 2 classes: backend and frontend
// here, for simplicity, we have both in one
//
struct StateCore {
    bool isInitialized = false;
    float fontScale = 1.0f;

    ImVec2 cardSize;
    ImVec2 handSize;
    float cardPadding;
    float scoreFrame;
    bool showFinishedCard = false;

    Game& game;

    // rendering
    Rendering rendering;

    // JS interface
    std::string dataDummy;
    std::string dataClipboard;
    std::string dataURL;

    bool showCircle = true;

    StateCore(Game& game);

    //
    // helper methods
    //

    // add any logic that should happen upon window resize
    void onWindowResize();

    // update the dataDummy member - to be consumed by the JS layer
    void updateDataDummy();

    //
    // UI methods
    //

    // call this once upon start of the program
    void init(float fontScale);

    // main render function
    void render();

    // called before rendering the new frame
    bool updatePre();

    // called after rendering each frame
    bool updatePost();

    // call this once upon exit of the program
    void deinitMain();

    void drawCard(Player* player, size_t cardIndex, ImVec2 pos, const ImVec2 &dir);

    void drawCardBack(const ImVec2 &pos, const ImVec2 &dir);
    void drawHand(Player* player, ImVec2 pos, const ImVec2 &dir);
    void drawToepButton(const ImVec2 &pos, const ImVec2 &dir);

    void cardsInPlay(ImVec2 pos);

    void drawPlayer(Player *player, const ImVec2 &dir);

    std::string tally(size_t score, bool horizontal);
};

