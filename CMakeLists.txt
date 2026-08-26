cmake_minimum_required(VERSION 3.20)
project(raylib-demo C)

set(CMAKE_C_STANDARD 23)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)

include(FetchContent)

FetchContent_Declare(raylib
  GIT_REPOSITORY https://github.com/raysan5/raylib
  GIT_TAG 6.0
  GIT_SHALLOW TRUE
  SYSTEM
)
FetchContent_MakeAvailable(raylib)

add_executable(raylib-demo src/main.c)
target_link_libraries(raylib-demo PRIVATE raylib)

# Some sexy warnings
target_compile_options(raylib-demo PRIVATE
    # -Werror
    -Wall
    -Wextra
    # -Wpedantic
    -Wconversion
    -Wsign-conversion
    -Wshadow
    -Wdouble-promotion
    -Wswitch-enum
    -Wnull-dereference
    -Wformat=2
    -Wstrict-prototypes
    # -Wmissing-prototypes
    # -Wmissing-declarations
    -Wvla
    -Wcast-align
    -Wcast-qual
    -Wundef
    -Wredundant-decls
)

add_compile_options(-O3 -flto=full)
add_link_options(-flto=full)
