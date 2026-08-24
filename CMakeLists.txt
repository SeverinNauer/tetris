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

if (EMSCRIPTEN)
    set_target_properties(raylib-demo
        PROPERTIES
            OUTPUT_NAME "index"
            SUFFIX ".html"
    )

    target_link_options(raylib-demo
        PRIVATE
            -sASYNCIFY
            -sALLOW_MEMORY_GROWTH=1
            -sGROWABLE_ARRAYBUFFERS=0
            -sINITIAL_MEMORY=134217728
            -sFORCE_FILESYSTEM=1
            -sMINIFY_HTML=0
            --shell-file "${CMAKE_SOURCE_DIR}/web/shell.html"
    )
endif()

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

target_compile_options(raylib-demo PRIVATE
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-flto>
)
target_link_options(raylib-demo PRIVATE
    $<$<CONFIG:Release>:-Os>
    $<$<CONFIG:Release>:-flto>
)
