cmake_minimum_required(VERSION 3.20)
project(raylib-demo C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Pass Nix compiler flags so clangd can find includes in compile_commands.json
if(DEFINED ENV{NIX_CFLAGS_COMPILE})
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} $ENV{NIX_CFLAGS_COMPILE}")
endif()

find_package(raylib REQUIRED)

add_executable(raylib-demo src/main.c)
target_link_libraries(raylib-demo PRIVATE raylib)
