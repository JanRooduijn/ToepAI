# ToepAI
A C++ implementation of the Dutch game Toepen.
![Screenshot of ToepAI](ss.png?raw=true "Sreenshot of ToepAI")

## Description
[Toepen](https://en.wikipedia.org/wiki/Toepen) is a popular Dutch card game. It is a relatively simple game, played with only 32 cards. Similar to Poker, it also has an element of bluff. ToepAI is an implementation of Toepen in C++ with both a backend and a frontend. For the graphics, it uses Dear ImGui and OpenGL, which can be compiled to WebAssemply using Emscripten. 

The bottom Player is controlled by the user, the other players all follow a heuristic defined in `ai.cpp`. Although the code is more flexible, the following options are currently hardcoded:
- There are 4 players
- Only the human player can _toep_ (a.k.a. raise)
- The maximum wager is 3 points

[Try it out here!](https://janrooduijn.github.io/ToepAI/)

## Todo
- Implement _vuile was_
- Also allow the computer players to _toep_
- Implement _overtoepen_ (a.k.a reraising)
- Add unit tests
- Guide the AI by a deep learning model, rather than the simple heuristics used presently

## Build native
```bash
# clone repo and build
git clone https://github.com/jmwrooduijn/toepAI --recursive
cd toepAI && mkdir build && cd build
cmake ..
make -j4

# run the app
./bin/toepAI-app
```

## Build web

```bash
# init emscripten
source /path/to/emsdk/emsdk_env.sh

# clone repo and build
git clone https://github.com/jmwrooduijn/toepAI --recursive
cd toepAI && mkdir build-em && cd build-em
emcmake cmake ..
make -j4

# update web server
cp ./bin/toepAI-app.*        /path/to/www/html/
cp ./bin/toepAI-app-public/* /path/to/www/html/
```

## Acknowledgements
ToepAI is built on top of the [GGWeb template](https://github.com/ggerganov/ggweb) by Georgi Gerganov. 