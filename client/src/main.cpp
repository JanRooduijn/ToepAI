#include "../include/build-timestamp-tmpl.h"
#include "../include/common.h"

#include "../include/state-sdl.h"
#include "../include/state-core.h"

#include "../include/icons-font-awesome.h"

#include "../../shared/include/game.h"

#include "imgui-extra/imgui_impl.h"

#include <SDL.h>

#include <string>
#include <functional>
#include <iostream>

#ifdef __EMSCRIPTEN__
#include <emscripten.h>
#include <emscripten/bind.h>
#else
#define EMSCRIPTEN_KEEPALIVE
#endif

namespace {

//
// Constants
//

// this improves the quality of the font at the expense of a memory and load time
constexpr auto kFontScale = 3.00f;

//
// App Interface
//

struct AppInterface {
    std::function<bool()>                     doInit;
    std::function<void(int, int)>             setWindowSize;
    std::function<void(const std::string & )> setData;
    std::function<std::string()>              getData;
    std::function<std::string()>              getClipboard;
    std::function<std::string()>              getURL;

    std::function<bool()> mainLoop;

    bool init(const StateSDL &stateSDL, StateCore &stateCore);

private:
    // setWindowSize
    int lastX = -1;
    int lastY = -1;
} g_appInterface;

#ifdef __EMSCRIPTEN__

// need this wrapper of the main loop for the emscripten_set_main_loop_arg() call
void mainLoop(void *) {
    g_appInterface.mainLoop();
}

//
// JS interface
//

// These functions are used to pass data back and forth between the JS and the C++ code
// using the app interface

EMSCRIPTEN_BINDINGS(ggweb) {
    emscripten::function("doInit",        emscripten::optional_override([]() -> int                   { return g_appInterface.doInit(); }));
    emscripten::function("setWindowSize", emscripten::optional_override([](int sizeX, int sizeY)      { g_appInterface.setWindowSize(sizeX, sizeY); }));
    emscripten::function("setData",       emscripten::optional_override([](const std::string & input) { g_appInterface.setData(input); }));
    emscripten::function("getData",       emscripten::optional_override([]() -> std::string           { return g_appInterface.getData(); }));
    emscripten::function("getClipboard",  emscripten::optional_override([]() -> std::string           { return g_appInterface.getClipboard(); }));
    emscripten::function("getURL",        emscripten::optional_override([]() -> std::string           { return g_appInterface.getURL(); }));
}

#endif

bool AppInterface::init(const StateSDL& stateSDL, StateCore& stateCore) {
    doInit = [&]() {
        stateCore.init(kFontScale);

        return true;
    };

    setWindowSize = [&](int sizeX, int sizeY) {
        if (lastX == sizeX && lastY == sizeY) {
            return;
        }

        lastX = sizeX;
        lastY = sizeY;

        SDL_SetWindowSize(stateSDL.window, sizeX, sizeY);

        stateCore.onWindowResize();
        stateCore.rendering.isAnimating = true;
    };

    setData = [&](const std::string & data) {
        std::cout << "Received some data from the JS layer: " << data.c_str() << std::endl;
    };

    getData = [&]() {
        const auto res = stateCore.dataDummy;
        stateCore.dataDummy.clear();
        return res;
    };

    getClipboard = [&]() {
        const auto res = stateCore.dataClipboard;
        stateCore.dataClipboard.clear();
        return res;
    };

    getURL = [&]() {
        const auto res = stateCore.dataURL;
        stateCore.dataURL.clear();
        return res;
    };

    mainLoop = [&]() {
        auto & nUpdates = stateCore.rendering.nUpdates;

        // framerate throtling when idle
        {
            --nUpdates;
            if (nUpdates < -30) nUpdates = 0;
            if (stateCore.rendering.isAnimating) nUpdates = std::max(nUpdates, 2);

            if (stateCore.isInitialized == false) {
                return true;
            }
        }

        // process window events
        {
            SDL_Event event;
            while (SDL_PollEvent(&event)) {
                nUpdates = std::max(nUpdates, 5);
                ImGui_ProcessEvent(&event);
                if (event.type == SDL_QUIT) return false;
                if (event.type == SDL_WINDOWEVENT && event.window.event == SDL_WINDOWEVENT_CLOSE && event.window.windowID == SDL_GetWindowID(stateSDL.window)) return false;
            }
        }

        // update game
        {
            std::lock_guard lock(stateCore.gameMutex_);
            stateCore.game.update();

        }

        // update + render
        {
            if (stateCore.updatePre() == false) {
                return false;
            }

            if (nUpdates >= 0) {
                if (ImGui::NewFrame(stateSDL.window) == false) {
                    return false;
                }

                stateCore.render();

                if (ImGui::EndFrame(stateSDL.window) == false) {
                    return false;
                }
            }

            if (stateCore.updatePost() == false) {
                return false;
            }
        }

        return true;
    };

    return true;
}

}

int main([[maybe_unused]] int argc, [[maybe_unused]] char** argv) {
    std::cout << "Build time: " << BUILD_TIMESTAMP << std::endl;

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        std::cerr << "Error: failed to initialize SDL. Reason: " << SDL_GetError() << std::endl;
        return -1;
    }

    // initalize the game
    Game game(4, 4, false);

    StateCore stateCore(game);
    StateSDL stateSDL = { .windowX = 1200, .windowY = 800, };

    // initialize SDL + ImGui
    {
        if (stateSDL.initWindow("ToepAI") == false) {
            std::cerr << "Error: failed to initialize SDL window." << std::endl;
            return -2;
        }

        if (stateSDL.initImGui(
                        kFontScale,
                        {
                            // add fonts to be loaded
                            { .filename = "DroidSans.ttf", .size = 13.0f * kFontScale, },
                            //{ .filename = "materialdesignicons-webfont.ttf", .size = 14.0f*kFontScale, .merge = true, .rangeMin = ICON_MIN_MDI, .rangeMax = ICON_MAX_MDI, },

                            // { .filename = "fontawesome-webfont.ttf", .size = 14.0f*kFontScale, .merge = false, .rangeMin = ICON_MIN_FA, .rangeMax = ICON_MAX_FA, },
                            //{ .filename = "some-cool-font.ttf", .size = 14.0f*kFontScale, .merge = false, .rangeMin = ..., .rangeMax = ..., },
                        }) == false) {
            std::cerr << "Error: failed to initialize ImGui." << std::endl;
            return -3;
        }
    }

    // initialize the application interface
    if (g_appInterface.init(stateSDL, stateCore) == false) {
        std::cerr << "Error: failed to initialize app interface." << std::endl;
        return -4;
    }


#ifdef __EMSCRIPTEN__
    // main - emscripten
    {
        emscripten_set_main_loop_arg(mainLoop, NULL, 0, true);
    }
#else
    // main - native
    {
        g_appInterface.setWindowSize(stateSDL.windowX, stateSDL.windowY);

        if (g_appInterface.doInit() == false) {
            std::cerr << "Error: failed to initialize" << std::endl;
            return -5;
        }

        // Ugly hack that somehow fixes scaling issues on my MacBook
        SDL_SetWindowSize(stateSDL.window, stateSDL.windowX + 1, stateSDL.windowY + 1);  // Slight resize
        SDL_SetWindowSize(stateSDL.window, stateSDL.windowX, stateSDL.windowY);  // Restore original size

        // main loop
        while (true) {
            if (g_appInterface.mainLoop() == false) {
                std::cout << "Main loop exited" << std::endl;
                break;
            }

            // update window size
            {
                int sizeX = -1;
                int sizeY = -1;

                SDL_GetWindowSize(stateSDL.window, &sizeX, &sizeY);
                g_appInterface.setWindowSize(sizeX, sizeY);

                int fbWidth, fbHeight;
                SDL_GetWindowSizeInPixels(stateSDL.window, &fbWidth, &fbHeight);  // Get actual framebuffer size
            }
        }

        // cleanup
        {
            stateCore.deinitMain();
            stateSDL.deinitImGui();
            stateSDL.deinitWindow();
        }
    }
#endif

    return 0;
}
