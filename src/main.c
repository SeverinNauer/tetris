#include <raylib.h>
#include <raymath.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>

const int screenWidth = 800;
const int screenHeight = 450;
const int blockSize = 20;
const int matrixWidth = 10;
const int matrixHeight = 20;
const int basePoints[4] = {40, 100, 300, 1200};
uint32_t score = 0;
uint32_t linesCleared = 0;

Vector2 matrixPosition = {.x = 100, .y = 20};

typedef struct BoardBlock {
  bool isSet;
  Color color;
} BoardBlock;

typedef struct Tetromino {
  char name;
  uint16_t definitions[4];
  Color color;
} Tetromino;

Tetromino O = {.name = 'O',
               .definitions = {0b0000011001100000, 0b0000011001100000,
                               0b0000011001100000, 0b0000011001100000},
               .color = YELLOW};
Tetromino I = {.name = 'I',
               .definitions = {0b0000111100000000, 0b0010001000100010,
                               0b0000000011110000, 0b0100010001000100},
               .color = MAGENTA};
Tetromino J = {.name = 'J',
               .definitions = {0b1000111000000000, 0b0110010001000000,
                               0b0000111000100000, 0b0100010011000000},
               .color = BLUE};
Tetromino L = {.name = 'L',
               .definitions = {0b0010111000000000, 0b0100010001100000,
                               0b0000111010000000, 0b1100010001000000},
               .color = ORANGE};
Tetromino S = {.name = 'S',
               .definitions = {0b0110110000000000, 0b0100011000100000,
                               0b0000011011000000, 0b1000110001000000},
               .color = GREEN};
Tetromino Z = {.name = 'Z',
               .definitions = {0b1100011000000000, 0b0010011001000000,
                               0b0000110001100000, 0b0100110010000000},
               .color = RED};
Tetromino T = {.name = 'T',
               .definitions = {0b0100111000000000, 0b0100011001000000,
                               0b0000111001000000, 0b0100110001000000},
               .color = PURPLE};
void drawLine(float x, float y, float endX, float endY) {
  Vector2 start = {.x = x, .y = y};
  Vector2 end = {.x = endX, .y = endY};
  DrawLineEx(start, end, 2.0, BLACK);
}

void drawMatrix(Vector2 position, uint8_t width, uint8_t height) {
  for (int row = 0; row < height; row++) {
    for (int col = 0; col < width; col++) {
      Rectangle rec = {.width = blockSize,
                       .height = blockSize,
                       .x = position.x + (col * blockSize),
                       .y = position.y + (row * blockSize)};
      DrawRectangleRoundedLinesEx(rec, 0.1, 1, 1, GRAY);
    }
  }
  drawLine(position.x, position.y, position.x,
           position.y + (height * blockSize));
  drawLine(position.x + (width * blockSize), position.y,
           position.x + (width * blockSize), position.y + (height * blockSize));
  drawLine(position.x, position.y + (height * blockSize),
           position.x + (width * blockSize), position.y + (height * blockSize));
}

Vector2 absolutePositionFromGridPosition(Vector2 gridPosition) {
  Vector2 absolutePosition = {
      .x = matrixPosition.x + (gridPosition.x * blockSize),
      .y = matrixPosition.y + (gridPosition.y * blockSize)};
  return absolutePosition;
}

void drawBlock(Color color, Vector2 gridPosition) {
  Vector2 absolutePosition = absolutePositionFromGridPosition(gridPosition);
  Rectangle rect = {.x = absolutePosition.x,
                    .y = absolutePosition.y,
                    .height = blockSize,
                    .width = blockSize};
  DrawRectangleRec(rect, color);
  DrawRectangleLinesEx(rect, 1.0f, BLACK);
}

void calculateBlockPositions(const Tetromino *tetromino, Vector2 basePosition,
                             int rotation, Vector2 output[4]) {
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
        .x = basePosition.x + col,
        .y = basePosition.y + row,
    };
    output[positionCounter++] = blockPosition;
  }
}

void drawTetromino(Vector2 position, const Tetromino *tetromino, int rotation) {
  Vector2 blockPositions[4];
  calculateBlockPositions(tetromino, position, rotation, blockPositions);
  for (int i = 0; i < 4; i++) {
    Vector2 blockPosition = blockPositions[i];
    drawBlock(tetromino->color, blockPosition);
  }
}

void drawStack(BoardBlock stack[matrixHeight][matrixWidth]) {
  for (int i = 0; i < matrixHeight; i++) {
    for (int j = 0; j < matrixWidth; j++) {
      BoardBlock block = stack[i][j];
      Vector2 gridPosition = {.x = j, .y = i};
      if (block.isSet) {
        drawBlock(block.color, gridPosition);
      }
    }
  }
}

void drawScore(uint32_t score, int level, uint32_t linesCleared) {
  Vector2 scorePosition = {.x = matrixPosition.x + matrixWidth * blockSize + 50,
                           .y = matrixPosition.y + matrixHeight * blockSize -
                                150};
  char text[11];

  Color color = LIME;

  sprintf(text, "%i", score);
  DrawText("Score", scorePosition.x, scorePosition.y, 20, BLACK);
  DrawText(text, scorePosition.x, scorePosition.y + 20, 24, color);
  sprintf(text, "%i", level);
  DrawText("Level", scorePosition.x, scorePosition.y + 50, 20, BLACK);
  DrawText(text, scorePosition.x, scorePosition.y + 70, 24, color);
  sprintf(text, "%i", linesCleared);
  DrawText("Lines", scorePosition.x, scorePosition.y + 100, 20, BLACK);
  DrawText(text, scorePosition.x, scorePosition.y + 120, 24, color);
}

bool canMove(Vector2 position, const Tetromino *tetromino, int rotation,
             BoardBlock stack[matrixHeight][matrixWidth]) {
  Vector2 blockPositions[4];
  calculateBlockPositions(tetromino, position, rotation, blockPositions);
  for (int i = 0; i < 4; i++) {
    Vector2 blockPosition = blockPositions[i];
    if (blockPosition.x >= matrixWidth || blockPosition.x < 0)
      return false;
    if (blockPosition.y >= matrixHeight || blockPosition.y < 0)
      return false;
    if (stack[(int)blockPosition.y][(int)blockPosition.x].isSet) {
      return false;
    }
  }
  return true;
}

void updateStack(BoardBlock stack[matrixHeight][matrixWidth],
                 const Tetromino *tetromino, Vector2 position, int rotation) {
  Vector2 blocks[4];
  calculateBlockPositions(tetromino, position, rotation, blocks);

  for (int i = 0; i < 4; i++) {
    Vector2 position = blocks[i];
    BoardBlock block = {.isSet = true, .color = tetromino->color};
    stack[(int)position.y][(int)position.x] = block;
  }
}

bool isRowFull(BoardBlock stackRow[matrixWidth]) {
  for (int i = 0; i < matrixWidth; i++) {
    BoardBlock block = stackRow[i];
    if (!block.isSet)
      return false;
  }
  return true;
}

void shiftRowsDown(BoardBlock stack[matrixHeight][matrixWidth], int row) {
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

int clearLines(BoardBlock stack[matrixHeight][matrixWidth]) {
  int lineCount = 0;
  for (int i = matrixHeight - 1; i >= 0; i--) {
    if (isRowFull(stack[i])) {
      shiftRowsDown(stack, i);
      i++;
      lineCount++;
    }
  }
  return lineCount;
}

int getCurrentLevel() { return linesCleared / 10; }

void addToScore(int linesCount, int level) {
  if (linesCount <= 0 || linesCount > 4)
    return;
  score += basePoints[linesCount - 1] * (level + 1);
}

int main(void) {
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

  SetTargetFPS(60);

  int tetrominoIndex = GetRandomValue(0, 6);

  Tetromino current_tetromino = blockTypes[tetrominoIndex];

  Vector2 current_position = {.x = 4, .y = 0};

  float timer = 0.0f;
  int current_rotation = 0;

  while (!WindowShouldClose()) {

    timer += GetFrameTime();

    if (timer >= 0.7f) {
      Vector2 positionToTest = {.x = current_position.x,
                                .y = current_position.y + 1};
      if (canMove(positionToTest, &current_tetromino, current_rotation,
                  stack)) {
        current_position.y += 1.0f;
      } else {
        updateStack(stack, &current_tetromino, current_position,
                    current_rotation);
        current_rotation = 0;
        int rIndex = GetRandomValue(0, 6);

        current_tetromino = blockTypes[rIndex];
        current_position.x = 4;
        current_position.y = 0;
        int linesClearedNow = clearLines(stack);
        addToScore(linesClearedNow, getCurrentLevel());
        linesCleared += linesClearedNow;
      }
      timer -= 0.7f;
    }

    if (IsKeyPressed(KEY_UP)) {
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

    if (IsKeyPressed(KEY_RIGHT)) {
      Vector2 positionToTest = {.x = current_position.x + 1,
                                .y = current_position.y};
      if (canMove(positionToTest, &current_tetromino, current_rotation,
                  stack)) {
        current_position.x += 1.0f;
      }
    }

    if (IsKeyPressed(KEY_DOWN)) {
      Vector2 positionToTest = {.x = current_position.x,
                                .y = current_position.y + 1};
      if (canMove(positionToTest, &current_tetromino, current_rotation,
                  stack)) {
        current_position.y += 1.0f;
        score += 1;
      }
    }

    if (IsKeyPressed(KEY_LEFT)) {
      Vector2 positionToTest = {.x = current_position.x - 1,
                                .y = current_position.y};
      if (canMove(positionToTest, &current_tetromino, current_rotation,
                  stack)) {
        current_position.x -= 1.0f;
      }
    }

    BeginDrawing();

    ClearBackground(RAYWHITE);

    drawMatrix(matrixPosition, matrixWidth, matrixHeight);
    drawTetromino(current_position, &current_tetromino, current_rotation);
    drawStack(stack);
    drawScore(score, getCurrentLevel(), linesCleared);

    EndDrawing();
  }

  CloseWindow();

  return 0;
}
