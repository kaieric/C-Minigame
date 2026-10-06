cmake_minimum_required(VERSION 3.24)
project(Contamination LANGUAGES C CXX)

set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
include(FetchContent)
# Pin the dependency so everyone builds the same version.
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
FetchContent_Declare(raylib
    URL https://github.com/raysan5/raylib/archive/refs/tags/5.5.tar.gz
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    TIMEOUT 30
    INACTIVITY_TIMEOUT 15
)
FetchContent_MakeAvailable(raylib)

add_executable(contamination
    src/main.cpp
    src/app/GameLoop.cpp
    src/game/GameState.cpp
    src/input/Input.cpp
    src/render/Renderer.cpp
)
target_compile_features(contamination PRIVATE cxx_std_17)
target_include_directories(contamination PRIVATE src)
target_link_libraries(contamination PRIVATE raylib)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(contamination PRIVATE -Wall -Wextra -Wpedantic)
endif()

if(EMSCRIPTEN)
    set_target_properties(contamination PROPERTIES SUFFIX ".html")
    target_link_options(contamination PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1")
endif()
