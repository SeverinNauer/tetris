cmake_minimum_required(VERSION 3.20)
project(Tetris C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Can be removed now?
#
# # Pass Nix compiler flags so clangd can find includes in compile_commands.json
# if(DEFINED ENV{NIX_CFLAGS_COMPILE})
#   set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{NIX_CFLAGS_COMPILE}")
# endif()

include(FetchContent)

FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(raylib)

add_executable(raylib-demo src/main.c)
target_link_libraries(raylib-demo PRIVATE raylib)
