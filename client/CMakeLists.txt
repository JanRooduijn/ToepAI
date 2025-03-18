if (GGWEB_ALL_WARNINGS)
    if (CMAKE_COMPILER_IS_GNUCC OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
        set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic")
    else()
        # todo : windows
    endif()
endif()

#
## Main
set(TARGET toepAI-client)
set(CMAKE_CXX_STANDARD 20)

find_package(Boost REQUIRED COMPONENTS system filesystem)
include_directories(${Boost_INCLUDE_DIRS})

# Set optimisaton level to 0 for profiling
#set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O0")
#set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fno-omit-frame-pointer -mno-omit-leaf-frame-pointer")

if(APPLE)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -stdlib=libc++ -I/Library/Developer/CommandLineTools/SDKs/MacOSX15.2.sdk/usr/include/c++/v1")
    set(CMAKE_OSX_ARCHITECTURES "arm64")
endif()

add_executable(${TARGET}
        src/main.cpp
        src/common.cpp
        src/state-sdl.cpp
        src/state-core.cpp
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
        ../shared/include/move.h
        src/client.cpp
        include/client.h
        src/connectionhandler.cpp
        include/connectionhandler.h
)

target_link_libraries(toepAI-client PRIVATE ${Boost_LIBRARIES})

target_include_directories(${TARGET} PUBLIC
        src
    ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}-extra/
    )

target_link_libraries(${TARGET} PRIVATE
    imgui-sdl2
    ${CMAKE_DL_LIBS}
    )



if (EMSCRIPTEN)
    unset(EXTRA_FLAGS)
    if (GGWEB_WASM_SINGLE_FILE)
        set(EXTRA_FLAGS "-s SINGLE_FILE=1")
        message(STATUS "Embedding WASM inside .js")

    endif()

    set_target_properties(${TARGET} PROPERTIES LINK_FLAGS " \
        --preload-file ${PROJECT_SOURCE_DIR}/fonts@/ \
        ${EXTRA_FLAGS} \
    ")

    configure_file(public/index-tmpl.html ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}-public/index.html @ONLY)
    configure_file(public/style.css ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}-public/style.css  COPYONLY)
    configure_file(public/helpers.js ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/${TARGET}-public/helpers.js COPYONLY)
endif()


if (NOT EMSCRIPTEN)
    file(COPY ${PROJECT_SOURCE_DIR}/client/fonts
        DESTINATION ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}/fonts)
endif()
