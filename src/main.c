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

constexpr int screenWidth = 800;
constexpr int screenHeight = 450;
constexpr int blockSize = 20;
constexpr int matrixWidth = 10;
constexpr int matrixHeight = 20;
constexpr uint32_t basePoints[4] = {40, 100, 300, 1200};

// NOTE: I added a underline suffix to the following global variable to prevent shadowing
uint32_t score_ = 0;
uint32_t linesCleared_ = 0;

Vector2 matrixPosition_ = {.x = 100, .y = 20};

typedef struct
{
    bool isSet;
    Color color;
} BoardBlock;

typedef struct
{
    char name;
    uint16_t definitions[4];
    Color color;
} Tetromino;

Tetromino O = {
  .name = 'O',
  .definitions = {0b0000011001100000, 0b0000011001100000, 0b0000011001100000, 0b0000011001100000},
  .color = YELLOW
};
Tetromino I = {
  .name = 'I',
  .definitions = {0b0000111100000000, 0b0010001000100010, 0b0000000011110000, 0b0100010001000100},
  .color = MAGENTA
};
Tetromino J = {
  .name = 'J',
  .definitions = {0b1000111000000000, 0b0110010001000000, 0b0000111000100000, 0b0100010011000000},
  .color = BLUE
};
Tetromino L = {
  .name = 'L',
  .definitions = {0b0010111000000000, 0b0100010001100000, 0b0000111010000000, 0b1100010001000000},
  .color = ORANGE
};
Tetromino S = {
  .name = 'S',
  .definitions = {0b0110110000000000, 0b0100011000100000, 0b0000011011000000, 0b1000110001000000},
  .color = GREEN
};
Tetromino Z = {
  .name = 'Z',
  .definitions = {0b1100011000000000, 0b0010011001000000, 0b0000110001100000, 0b0100110010000000},
  .color = RED
};
Tetromino T = {
  .name = 'T',
  .definitions = {0b0100111000000000, 0b0100011001000000, 0b0000111001000000, 0b0100110001000000},
  .color = PURPLE
};

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

void calculateBlockPositions(const Tetromino* tetromino,
                             Vector2 basePosition,
                             int rotation,
                             Vector2 output[4])
{
    uint16_t mask = 0b1000000000000000;
    int positionCounter = 0;
    for (int i = 0; i < 16; i++) {
        uint16_t exists = tetromino->definitions[rotation] & mask;
        mask >>= 1;
        if (!exists) {
            continue;
        }

        int col = i % 4;
        int row = i / 4;
        Vector2 blockPosition = {
          .x = basePosition.x + (float)col,
          .y = basePosition.y + (float)row,
        };
        output[positionCounter++] = blockPosition;
    }
}

void drawTetromino(Vector2 position,
                   const Tetromino* tetromino,
                   int rotation,
                   Vector2 matrixPosition)
{
    Vector2 blockPositions[4];
    calculateBlockPositions(tetromino, position, rotation, blockPositions);
    for (int i = 0; i < 4; i++) {
        Vector2 blockPosition = blockPositions[i];
        drawBlock(tetromino->color, blockPosition, matrixPosition);
    }
}

void drawStack(BoardBlock stack[matrixHeight][matrixWidth])
{
    for (int i = 0; i < matrixHeight; i++) {
        for (int j = 0; j < matrixWidth; j++) {
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

void drawPreview(Tetromino* previewTetromino)
{
    const int previewCols = 4;
    const int previewRows = 2;
    const int padding = 10;

    Vector2 previewPosition = {.x = matrixPosition_.x + matrixWidth * blockSize + 50,
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

    Vector2 zeroPosition = {};
    drawTetromino(zeroPosition, previewTetromino, 0, localMatrix);
}

void drawScore(uint32_t score, unsigned level, uint32_t linesCleared)
{
    Vector2 scorePosition = {.x = matrixPosition_.x + matrixWidth * blockSize + 50,
                             .y = matrixPosition_.y + matrixHeight * blockSize - 150};
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

[[nodiscard]]
bool canMove(Vector2 position,
             const Tetromino* tetromino,
             int rotation,
             BoardBlock stack[matrixHeight][matrixWidth])
{
    Vector2 blockPositions[4];
    calculateBlockPositions(tetromino, position, rotation, blockPositions);
    for (int i = 0; i < 4; i++) {
        Vector2 blockPosition = blockPositions[i];

        if (blockPosition.x >= matrixWidth || blockPosition.x < 0) {
            return false;
        }

        if (blockPosition.y >= matrixHeight || blockPosition.y < 0) {
            return false;
        }

        if (stack[(int)blockPosition.y][(int)blockPosition.x].isSet) {
            return false;
        }
    }
    return true;
}

void updateStack(BoardBlock stack[matrixHeight][matrixWidth],
                 const Tetromino* tetromino,
                 Vector2 position,
                 int rotation)
{
    Vector2 blocks[4];
    calculateBlockPositions(tetromino, position, rotation, blocks);

    for (int i = 0; i < 4; i++) {
        Vector2 pos = blocks[i];

        BoardBlock block = {
          .isSet = true,
          .color = tetromino->color,
        };

        stack[(int)pos.y][(int)pos.x] = block;
    }
}

[[nodiscard]]
bool isRowFull(BoardBlock stackRow[matrixWidth])
{
    for (int i = 0; i < matrixWidth; i++) {
        BoardBlock block = stackRow[i];
        if (!block.isSet) {
            return false;
        }
    }
    return true;
}

void shiftRowsDown(BoardBlock stack[matrixHeight][matrixWidth], int row)
{
    for (int i = row; i > 0; i--) {
        for (int j = 0; j < matrixWidth; j++) {
            stack[i][j].isSet = stack[i - 1][j].isSet;
            stack[i][j].color = stack[i - 1][j].color;
        }
    }

    for (int i = 0; i < matrixWidth; i++) {
        stack[0][i].isSet = false;
        stack[0][i].color = BLANK;
    }
}

[[nodiscard]]
uint32_t clearLines(BoardBlock stack[matrixHeight][matrixWidth])
{
    uint32_t lineCount = 0;
    for (int i = matrixHeight - 1; i >= 0; i--) {
        if (isRowFull(stack[i])) {
            shiftRowsDown(stack, i);
            i++;
            lineCount++;
        }
    }
    return lineCount;
}

[[nodiscard]]
uint32_t getCurrentLevel()
{
    return linesCleared_ / 10;
}

void addToScore(uint32_t linesCount, uint32_t level)
{
    if (linesCount <= 0 || linesCount > 4) {
        return;
    }
    score_ += basePoints[linesCount - 1] * (level + 1);
}

void lockAndRespawn(Tetromino* current,
                    Tetromino* next,
                    Vector2* position,
                    int* rotation,
                    BoardBlock stack[matrixHeight][matrixWidth],
                    uint32_t level,
                    Tetromino blockTypes[])
{
    updateStack(stack, current, *position, *rotation);
    *rotation = 0;
    const int rIndex = GetRandomValue(0, 6);
    *current = *next;
    *next = blockTypes[rIndex];
    position->x = 4;
    position->y = 0;

    const uint32_t linesClearedNow = clearLines(stack);
    addToScore(linesClearedNow, level);
    linesCleared_ += linesClearedNow;
}

int main(void)
{
    Tetromino blockTypes[] = {O, Z, J, L, S, T, I};

    BoardBlock stack[matrixHeight][matrixWidth];

    for (int i = 0; i < matrixHeight; i++) {
        for (int j = 0; j < matrixWidth; j++) {
            BoardBlock block = {.isSet = false, .color = BLANK};
            stack[i][j] = block;
        }
    }

    SetConfigFlags(FLAG_WINDOW_HIGHDPI);
    InitWindow(screenWidth, screenHeight, "Best Tetris Ever");
    InitAudioDevice(); // Initialize audio device

    Music music = LoadMusicStream("resources/music.mp3");
    PlayMusicStream(music);

    SetTargetFPS(240);

    int tetrominoIndex = GetRandomValue(0, 6);
    Tetromino current_tetromino = blockTypes[tetrominoIndex];

    tetrominoIndex = GetRandomValue(0, 6);
    Tetromino next_tetromino = blockTypes[tetrominoIndex];

    Vector2 current_position = {.x = 4, .y = 0};

    float timer = 0.0f;
    int current_rotation = 0;

    while (!WindowShouldClose()) {
        timer += GetFrameTime();
      
        UpdateMusicStream(music);

        uint32_t level = getCurrentLevel();
        if (IsKeyPressed(KEY_SPACE)) {
            uint32_t drop = 0;
            Vector2 positionToTest = {.x = current_position.x, .y = current_position.y + 1};
            while (canMove(positionToTest, &current_tetromino, current_rotation, stack)) {
                current_position.y += 1.0f;
                drop++;
                positionToTest.y += 1.0f;
            }
            score_ += drop;
            lockAndRespawn(&current_tetromino,
                           &next_tetromino,
                           &current_position,
                           &current_rotation,
                           stack,
                           level,
                           blockTypes);
            timer = 0;
        }

        if (IsKeyPressed(KEY_DOWN) || IsKeyPressed(KEY_J)) {
            Vector2 positionToTest = {.x = current_position.x, .y = current_position.y + 1};
            if (canMove(positionToTest, &current_tetromino, current_rotation, stack)) {
                current_position.y += 1.0f;
                score_ += 1;
            } else {
                lockAndRespawn(&current_tetromino,
                               &next_tetromino,
                               &current_position,
                               &current_rotation,
                               stack,
                               level,
                               blockTypes);
            }
            timer = 0;
        }

        const float currentDropTime =
          (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J)) ? 0.08f : getCurrentDropTime(level);

        if (timer >= currentDropTime) {
            Vector2 positionToTest = {.x = current_position.x, .y = current_position.y + 1};
            if (canMove(positionToTest, &current_tetromino, current_rotation, stack)) {
                if (IsKeyDown(KEY_DOWN) || IsKeyDown(KEY_J)) {
                    score_ += 1;
                }
                current_position.y += 1.0f;
            } else {
                lockAndRespawn(&current_tetromino,
                               &next_tetromino,
                               &current_position,
                               &current_rotation,
                               stack,
                               level,
                               blockTypes);
            }
            timer = 0;
        }

        if (IsKeyPressed(KEY_UP) || IsKeyPressed(KEY_K)) {
            int next_rotation = (current_rotation + 1) % 4;
            if (canMove(current_position, &current_tetromino, next_rotation, stack)) {
                current_rotation = next_rotation;
            }
        }

        if (IsKeyPressed(KEY_RIGHT_CONTROL)) {
            int next_rotation = (current_rotation + 4 - 1) % 4;
            if (canMove(current_position, &current_tetromino, next_rotation, stack)) {
                current_rotation = next_rotation;
            }
        }

        if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_L)) {
            Vector2 positionToTest = {.x = current_position.x + 1, .y = current_position.y};
            if (canMove(positionToTest, &current_tetromino, current_rotation, stack)) {
                current_position.x += 1.0f;
            }
        }

        if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_H)) {
            Vector2 positionToTest = {.x = current_position.x - 1, .y = current_position.y};
            if (canMove(positionToTest, &current_tetromino, current_rotation, stack)) {
                current_position.x -= 1.0f;
            }
        }

        BeginDrawing();

        ClearBackground(RAYWHITE);

        drawMatrix(matrixPosition_, matrixWidth, matrixHeight);
        drawTetromino(current_position, &current_tetromino, current_rotation, matrixPosition_);
        drawPreview(&next_tetromino);
        drawStack(stack);
        drawScore(score_, level, linesCleared_);

        EndDrawing();
    }
    UnloadMusicStream(music);
    loseAudioDevice();
    CloseWindow();

    return 0;
}
