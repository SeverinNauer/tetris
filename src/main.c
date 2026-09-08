#include "draw.c"

#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>

// calculated drop times for level 0 to 29 based on nes
// speed stays the same after level 29
static const float dropTime[] = {
  48.0f / 60.0f, // Level 0
  43.0f / 60.0f, // Level 1
  38.0f / 60.0f, // Level 2
  33.0f / 60.0f, // Level 3
  28.0f / 60.0f, // Level 4
  23.0f / 60.0f, // Level 5
  18.0f / 60.0f, // Level 6
  13.0f / 60.0f, // Level 7
  8.0f / 60.0f,  // Level 8
  6.0f / 60.0f,  // Level 9
  5.0f / 60.0f,  // Level 10
  5.0f / 60.0f,  // Level 11
  5.0f / 60.0f,  // Level 12
  4.0f / 60.0f,  // Level 13
  4.0f / 60.0f,  // Level 14
  4.0f / 60.0f,  // Level 15
  3.0f / 60.0f,  // Level 16
  3.0f / 60.0f,  // Level 17
  3.0f / 60.0f,  // Level 18
  2.0f / 60.0f,  // Level 19
  2.0f / 60.0f,  // Level 20
  2.0f / 60.0f,  // Level 21
  2.0f / 60.0f,  // Level 22
  2.0f / 60.0f,  // Level 23
  2.0f / 60.0f,  // Level 24
  2.0f / 60.0f,  // Level 25
  2.0f / 60.0f,  // Level 26
  2.0f / 60.0f,  // Level 27
  2.0f / 60.0f,  // Level 28
  1.0f / 60.0f   // Level 29+
};

const int screenWidth = 800;
const int screenHeight = 450;

[[nodiscard]]
float getCurrentDropTime(uint32_t level)
{
    level = level <= 29 ? level : 29;
    return dropTime[level];
}

int main(void)
{
    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "Best Tetris Ever");
    InitAudioDevice(); // Initialize audio device

    Music music = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(music);

    SetTargetFPS(240);

    GameState state = {};
    initGameState(&state);

    float timer = 0.0f;

    while (!WindowShouldClose()) {
        timer += GetFrameTime();

        UpdateMusicStream(music);

        const bool softDrop = IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J);

        if (IsKeyPressed(KEY_SPACE)) {
            uint32_t drop = 0;
            while (moveBlock(&state, (Position){.y = 1})) {
                drop++;
            }
            state.score += drop;
            lockAndRespawn(&state);
            timer = 0;
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) {
            if (moveBlock(&state, (Position){.y = 1})) {
                state.score += 1;
            } else {
                lockAndRespawn(&state);
            }
            timer = 0;
        }

        const float currentDropTime = softDrop ? 0.08f
                                               : getCurrentDropTime(getCurrentLevel(&state));

        if (timer >= currentDropTime) {
            if (moveBlock(&state, (Position){.y = 1})) {
                if (softDrop) {
                    state.score += 1;
                }
            } else {
                lockAndRespawn(&state);
            }
            timer = 0;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_K)) {
            rotateBlock(&state, 1);
        }

        if (IsKeyPressed(KEY_RIGHT_CONTROL)) {
            rotateBlock(&state, -1);
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_L)) {
            moveBlock(&state, (Position){.x = 1});
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_H)) {
            moveBlock(&state, (Position){.x = -1});
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawMatrix(matrixPosition_, MATRIX_WIDTH, MATRIX_HEIGHT);
        drawTetromino(state.current_tetromino.tetromino,
                      state.current_tetromino.rotation,
                      state.current_tetromino.position,
                      matrixPosition_);
        drawPreview(state.next_tetromino);
        drawStack(state.board);
        drawScore(state.score, getCurrentLevel(&state), state.linesCleared);

        EndDrawing();
    }
    UnloadMusicStream(music);
    CloseAudioDevice();
    CloseWindow();

    return 0;
}