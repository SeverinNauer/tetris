#include "state.c"

#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

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
const int blockSize = 20;

// NOTE: I added a underline suffix to the following global variable to prevent shadowing
uint32_t score_ = 0;
uint32_t linesCleared_ = 0;

Vector2 matrixPosition_ = {.x = 100, .y = 20};

[[nodiscard]]
float getCurrentDropTime(uint32_t level)
{
    level = level <= 29 ? level : 29;
    return dropTime[level];
}

void drawLine(float x, float y, float endX, float endY)
{
    Vector2 start = {.x = x, .y = y};
    Vector2 end = {.x = endX, .y = endY};
    DrawLineEx(start, end, 2.0, BLACK);
}

void drawMatrix(Vector2 position, uint8_t width, uint8_t height)
{
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            Rectangle rec = {.width = blockSize,
                             .height = blockSize,
                             .x = position.x + (float)(col * blockSize),
                             .y = position.y + (float)(row * blockSize)};
            DrawRectangleRoundedLinesEx(rec, 0.1f, 1, 1, GRAY);
        }
    }
    drawLine(position.x, position.y, position.x, position.y + (height * blockSize));
    drawLine(position.x + (width * blockSize),
             position.y,
             position.x + (width * blockSize),
             position.y + (height * blockSize));
    drawLine(position.x,
             position.y + (height * blockSize),
             position.x + (width * blockSize),
             position.y + (height * blockSize));
}

Vector2 absolutePositionFromGridPosition(Vector2 gridPosition, Vector2 matrixPosition)
{
    Vector2 absolutePosition = {.x = matrixPosition.x + (gridPosition.x * blockSize),
                                .y = matrixPosition.y + (gridPosition.y * blockSize)};
    return absolutePosition;
}

void drawBlock(Color color, Vector2 gridPosition, Vector2 matrixPosition)
{
    Vector2 absolutePosition = absolutePositionFromGridPosition(gridPosition, matrixPosition);
    Rectangle rect = {
      .x = absolutePosition.x, .y = absolutePosition.y, .height = blockSize, .width = blockSize};
    DrawRectangleRec(rect, color);
    DrawRectangleLinesEx(rect, 1.0f, BLACK);
}

void drawTetromino(const Tetromino* tetromino,
                   int rotation,
                   Position position,
                   Vector2 matrixPosition)
{
    Vector2 blockPositions[4];
    uint16_t definition = tetromino->definitions[rotation];
    calculateBlockPositions(position, definition, blockPositions);
    for (int i = 0; i < 4; i++) {
        Vector2 blockPosition = blockPositions[i];
        drawBlock(tetromino->color, blockPosition, matrixPosition);
    }
}

void drawStack(BoardBlock stack[MATRIX_HEIGHT][MATRIX_WIDTH])
{
    for (int i = 0; i < MATRIX_HEIGHT; i++) {
        for (int j = 0; j < MATRIX_WIDTH; j++) {
            BoardBlock block = stack[i][j];
            Vector2 gridPosition = {.x = (float)j, .y = (float)i};
            if (block.isSet) {
                drawBlock(block.color, gridPosition, matrixPosition_);
            }
        }
    }
}

void getPieceBounds(const Tetromino* tetromino,
                    int rotation,
                    int* minCol,
                    int* minRow,
                    int* maxCol,
                    int* maxRow)
{
    *minCol = 4;
    *minRow = 4;
    *maxCol = -1;
    *maxRow = -1;
    uint16_t mask = 0b1000000000000000;
    for (int i = 0; i < 16; i++) {
        uint16_t exists = tetromino->definitions[rotation] & mask;
        mask >>= 1;
        if (!exists) {
            continue;
        }
        int col = i % 4;
        int row = i / 4;
        if (col < *minCol) {
            *minCol = col;
        }
        if (col > *maxCol) {
            *maxCol = col;
        }
        if (row < *minRow) {
            *minRow = row;
        }
        if (row > *maxRow) {
            *maxRow = row;
        }
    }
}

void drawPreview(const Tetromino* previewTetromino)
{
    const int previewCols = 4;
    const int previewRows = 2;
    const int padding = 10;

    Vector2 previewPosition = {.x = matrixPosition_.x + MATRIX_WIDTH * blockSize + 50,
                               .y = matrixPosition_.y + 10};

    int panelWidth = previewCols * blockSize + 2 * padding;
    int panelHeight = previewRows * blockSize + 2 * padding + 25;
    Rectangle panel = {
      .x = previewPosition.x,
      .y = previewPosition.y,
      .width = (float)panelWidth,
      .height = (float)panelHeight,
    };

    DrawRectangleRounded(panel, 0.15f, 8, WHITE);
    DrawRectangleRoundedLinesEx(panel, 0.15f, 8, 1.5f, GRAY);
    DrawText("NEXT", (int)previewPosition.x + 10, (int)previewPosition.y + 10, 18, BLACK);

    int minCol, minRow, maxCol, maxRow;
    getPieceBounds(previewTetromino, 0, &minCol, &minRow, &maxCol, &maxRow);
    int pieceWidth = (maxCol - minCol + 1) * blockSize;
    int pieceHeight = (maxRow - minRow + 1) * blockSize;

    Vector2 pieceTopLeft = {
      .x = previewPosition.x + (float)padding + ((float)(previewCols * blockSize - pieceWidth) / 2),
      .y =
        previewPosition.y + (float)padding + ((float)(previewRows * blockSize - pieceHeight) / 2),
    };

    Vector2 localMatrix = {
      .x = pieceTopLeft.x - (float)(minCol * blockSize),
      .y = pieceTopLeft.y + 25 - (float)(minRow * blockSize),
    };

    Position zeroPosition = {};
    drawTetromino(previewTetromino, 0, zeroPosition, localMatrix);
}

void drawScore(uint32_t score, unsigned level, uint32_t linesCleared)
{
    Vector2 scorePosition = {.x = matrixPosition_.x + MATRIX_WIDTH * blockSize + 50,
                             .y = matrixPosition_.y + MATRIX_HEIGHT * blockSize - 150};
    char text[11];

    Color color = LIME;

    const int scorePositionX = (int)scorePosition.x;
    const int scorePositionY = (int)scorePosition.y;

    sprintf(text, "%i", score);
    DrawText("Score", scorePositionX, scorePositionY, 20, BLACK);
    DrawText(text, scorePositionX, scorePositionY + 20, 24, color);

    sprintf(text, "%i", level);
    DrawText("Level", scorePositionX, scorePositionY + 50, 20, BLACK);
    DrawText(text, scorePositionX, scorePositionY + 70, 24, color);

    sprintf(text, "%i", linesCleared);
    DrawText("Lines", scorePositionX, scorePositionY + 100, 20, BLACK);
    DrawText(text, scorePositionX, scorePositionY + 120, 24, color);
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

        if (IsKeyPressed(KEY_SPACE)) {
            uint32_t drop = 0;
            Position positionToTest = {.x = state.current_tetromino.position.x,
                                       .y = state.current_tetromino.position.y + 1};
            while (canMove(positionToTest, state.current_tetromino.rotation, &state)) {
                state.current_tetromino.position = positionToTest;
                drop++;
                positionToTest.y += 1;
            }
            score_ += drop;
            lockAndRespawn(&state);
            timer = 0;
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) {
            Position positionToTest = {.x = state.current_tetromino.position.x,
                                       .y = state.current_tetromino.position.y + 1};
            if (canMove(positionToTest, state.current_tetromino.rotation, &state)) {
                state.current_tetromino.position = positionToTest;
                score_ += 1;
            } else {
                lockAndRespawn(&state);
            }
            timer = 0;
        }

        const float currentDropTime = (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J))
                                      ? 0.08f
                                      : getCurrentDropTime(getCurrentLevel(&state));

        if (timer >= currentDropTime) {
            Position positionToTest = {.x = state.current_tetromino.position.x,
                                       .y = state.current_tetromino.position.y + 1};
            if (canMove(positionToTest, state.current_tetromino.rotation, &state)) {
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J)) {
                    score_ += 1;
                }
                state.current_tetromino.position = positionToTest;
            } else {
                lockAndRespawn(&state);
            }
            timer = 0;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_K)) {
            int next_rotation = (state.current_tetromino.rotation + 1) % 4;
            if (canMove(state.current_tetromino.position, next_rotation, &state)) {
                state.current_tetromino.rotation = next_rotation;
            }
        }

        if (IsKeyPressed(KEY_RIGHT_CONTROL)) {
            int next_rotation = (state.current_tetromino.rotation + 4 - 1) % 4;
            if (canMove(state.current_tetromino.position, next_rotation, &state)) {
                state.current_tetromino.rotation = next_rotation;
            }
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_L)) {
            Position positionToTest = {.x = state.current_tetromino.position.x + 1,
                                       .y = state.current_tetromino.position.y};
            if (canMove(positionToTest, state.current_tetromino.rotation, &state)) {
                state.current_tetromino.position = positionToTest;
            }
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_H)) {
            Position positionToTest = {.x = state.current_tetromino.position.x - 1,
                                       .y = state.current_tetromino.position.y};
            if (canMove(positionToTest, state.current_tetromino.rotation, &state)) {
                state.current_tetromino.position = positionToTest;
            }
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
