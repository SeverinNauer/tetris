#pragma once

#include "definition.c"

#include <stdint.h>

typedef struct
{
    const Tetromino* tetromino;
    int rotation;
    Position position;
} TetrominoState;

typedef struct
{
    TetrominoState current_tetromino;
    const Tetromino* next_tetromino;
    BoardBlock board[MATRIX_HEIGHT][MATRIX_WIDTH];
    uint32_t score;
    uint32_t linesCleared;
} GameState;

uint32_t getCurrentLevel(const GameState* state)
{
    return state->linesCleared / 10;
}

void addToScore(uint32_t linesCount, GameState* state)
{
    uint32_t level = getCurrentLevel(state);
    if (linesCount == 0 || linesCount > 4) {
        return;
    }
    state->score += basePoints[linesCount - 1] * (level + 1);
}

void calculateBlockPositions(Position position, uint16_t definition, Vector2 output[4])
{
    uint16_t mask = 0b1000000000000000;
    int positionCounter = 0;
    for (int i = 0; i < 16; i++) {
        uint16_t exists = definition & mask;
        mask >>= 1;
        if (!exists) {
            continue;
        }

        int col = i % 4;
        int row = i / 4;
        Vector2 blockPosition = {
          .x = (float)position.x + (float)col,
          .y = (float)position.y + (float)row,
        };
        output[positionCounter++] = blockPosition;
    }
}

void updateStack(GameState* state)
{
    Vector2 blocks[4];
    calculateBlockPositions(
      state->current_tetromino.position,
      state->current_tetromino.tetromino->definitions[state->current_tetromino.rotation],
      blocks);

    for (int i = 0; i < 4; i++) {
        Vector2 pos = blocks[i];

        BoardBlock block = {
          .isSet = true,
          .color = state->current_tetromino.tetromino->color,
        };

        state->board[(int)pos.y][(int)pos.x] = block;
    }
}

[[nodiscard]]
bool isRowFull(BoardBlock stackRow[MATRIX_WIDTH])
{
    for (int i = 0; i < MATRIX_WIDTH; i++) {
        BoardBlock block = stackRow[i];
        if (!block.isSet) {
            return false;
        }
    }
    return true;
}

void shiftRowsDown(GameState* state, int row)
{
    for (int i = row; i > 0; i--) {
        for (int j = 0; j < MATRIX_WIDTH; j++) {
            state->board[i][j].isSet = state->board[i - 1][j].isSet;
            state->board[i][j].color = state->board[i - 1][j].color;
        }
    }

    for (int i = 0; i < MATRIX_WIDTH; i++) {
        state->board[0][i].isSet = false;
        state->board[0][i].color = BLANK;
    }
}

uint32_t clearLines(GameState* state)
{
    uint32_t lineCount = 0;
    for (int i = MATRIX_HEIGHT - 1; i >= 0; i--) {
        if (isRowFull(state->board[i])) {
            shiftRowsDown(state, i);
            i++;
            lineCount++;
        }
    }
    return lineCount;
}

[[nodiscard]]
static const Tetromino* randomTetromino(void)
{
    const int bound = (int)(sizeof blockTypes / sizeof blockTypes[0]) - 1;
    return &blockTypes[GetRandomValue(0, bound)];
}

void spawnCurrent(GameState* state)
{
    state->current_tetromino.tetromino = state->next_tetromino;
    state->current_tetromino.rotation = 0;
    state->current_tetromino.position.x = 4;
    state->current_tetromino.position.y = 0;
    state->next_tetromino = randomTetromino();
}

void lockAndRespawn(GameState* state)
{
    updateStack(state);
    spawnCurrent(state);

    uint32_t linesClearedNow = clearLines(state);
    addToScore(linesClearedNow, state);
    state->linesCleared += linesClearedNow;
}

[[nodiscard]]
bool canMove(Position positionToTest, int rotationToTest, const GameState* state)
{
    Vector2 blockPositions[4];
    uint16_t definition = state->current_tetromino.tetromino->definitions[rotationToTest];
    calculateBlockPositions(positionToTest, definition, blockPositions);
    for (int i = 0; i < 4; i++) {
        Vector2 blockPosition = blockPositions[i];

        if (blockPosition.x >= MATRIX_WIDTH || blockPosition.x < 0) {
            return false;
        }

        if (blockPosition.y >= MATRIX_HEIGHT || blockPosition.y < 0) {
            return false;
        }

        if (state->board[(int)blockPosition.y][(int)blockPosition.x].isSet) {
            return false;
        }
    }
    return true;
}

bool moveBlock(GameState* state, Position delta)
{
    Position newPosition = {
      .x = state->current_tetromino.position.x + delta.x,
      .y = state->current_tetromino.position.y + delta.y,
    };
    if (!canMove(newPosition, state->current_tetromino.rotation, state)) {
        return false;
    }
    state->current_tetromino.position = newPosition;
    return true;
}

bool rotateBlock(GameState* state, int direction)
{
    int nextRotation = (state->current_tetromino.rotation + direction + 4) % 4;
    if (!canMove(state->current_tetromino.position, nextRotation, state)) {
        return false;
    }
    state->current_tetromino.rotation = nextRotation;
    return true;
}

void initGameState(GameState* state)
{
    for (int i = 0; i < MATRIX_HEIGHT; i++) {
        for (int j = 0; j < MATRIX_WIDTH; j++) {
            BoardBlock block = {.isSet = false, .color = BLANK};
            state->board[i][j] = block;
        }
    }
    state->next_tetromino = randomTetromino();
    spawnCurrent(state);
}
