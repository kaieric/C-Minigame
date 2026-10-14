cmake_minimum_required(VERSION 3.24)
project(FiniteYield LANGUAGES C CXX)

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

add_executable(finite_yield
    src/main.cpp
    src/app/GameLoop.cpp
    src/game/GameState.cpp
    src/game/ResourceUpdate.cpp
    src/input/Input.cpp
    src/render/Renderer.cpp
)
target_compile_features(finite_yield PRIVATE cxx_std_17)
target_include_directories(finite_yield PRIVATE src)
target_link_libraries(finite_yield PRIVATE raylib)
if(CMAKE_CXX_COMPILER_ID MATCHES "Clang|GNU")
    target_compile_options(finite_yield PRIVATE -Wall -Wextra -Wpedantic)
endif()

include(CTest)
if(BUILD_TESTING AND NOT EMSCRIPTEN)
    add_executable(resource_update_tests
        tests/ResourceUpdateTests.cpp
        src/game/GameState.cpp
        src/game/ResourceUpdate.cpp
    )
    target_compile_features(resource_update_tests PRIVATE cxx_std_17)
    target_include_directories(resource_update_tests PRIVATE src)
    add_test(NAME resource_updates COMMAND resource_update_tests)
endif()

if(EMSCRIPTEN)
    set_target_properties(finite_yield PROPERTIES SUFFIX ".html")
    target_link_options(finite_yield PRIVATE "-sUSE_GLFW=3" "-sALLOW_MEMORY_GROWTH=1")
    target_link_options(finite_yield PRIVATE
        "SHELL:--preload-file ${CMAKE_CURRENT_SOURCE_DIR}/assets@assets")
else()
    # Copy on every build, including when only an asset changed.
    add_custom_target(copy_assets ALL
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "$<TARGET_FILE_DIR:finite_yield>/assets"
        COMMENT "Copying terrain assets")
    add_dependencies(finite_yield copy_assets)
endif()
