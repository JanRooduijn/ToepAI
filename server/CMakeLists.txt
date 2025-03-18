cmake_minimum_required(VERSION 3.28)
project(ToepAI-server)

set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

set(TARGET toepAI-server)
add_executable(${TARGET} src/main.cpp
        src/server.cpp
        include/server.h
        src/gamehandler.cpp
        include/gamehandler.h
        ../shared/src/card.cpp
        ../shared/include/card.h
        ../shared/src/hand.cpp
        ../shared/include/hand.h
        ../shared/include/deck.h
        ../shared/src/deck.cpp
        ../shared/include/game.h
        ../shared/include/player.h
        ../shared/src/game.cpp
        ../shared/src/player.cpp
        ../shared/include/ai.h
        ../shared/src/ai.cpp
        ../shared/include/move.h)

target_link_libraries(toepAI-server PRIVATE ${Boost_LIBRARIES})
