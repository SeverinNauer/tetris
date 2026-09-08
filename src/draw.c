#pragma once

#include "state.c"

#include <raylib.h>
#include <stdint.h>
#include <stdio.h>

Vector2 matrixPosition_ = {.x = 100, .y = 20};

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
            Rectangle rec = {.width = BLOCK_SIZE,
                             .height = BLOCK_SIZE,
                             .x = position.x + (float)(col * BLOCK_SIZE),
                             .y = position.y + (float)(row * BLOCK_SIZE)};
            DrawRectangleRoundedLinesEx(rec, 0.1f, 1, 1, GRAY);
        }
    }
    drawLine(position.x, position.y, position.x, position.y + (height * BLOCK_SIZE));
    drawLine(position.x + (width * BLOCK_SIZE),
             position.y,
             position.x + (width * BLOCK_SIZE),
             position.y + (height * BLOCK_SIZE));
    drawLine(position.x,
             position.y + (height * BLOCK_SIZE),
             position.x + (width * BLOCK_SIZE),
             position.y + (height * BLOCK_SIZE));
}

Vector2 absolutePositionFromGridPosition(Vector2 gridPosition, Vector2 matrixPosition)
{
    Vector2 absolutePosition = {.x = matrixPosition.x + (gridPosition.x * BLOCK_SIZE),
                                .y = matrixPosition.y + (gridPosition.y * BLOCK_SIZE)};
    return absolutePosition;
}

void drawBlock(Color color, Vector2 gridPosition, Vector2 matrixPosition)
{
    Vector2 absolutePosition = absolutePositionFromGridPosition(gridPosition, matrixPosition);
    Rectangle rect = {
      .x = absolutePosition.x, .y = absolutePosition.y, .height = BLOCK_SIZE, .width = BLOCK_SIZE};
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

    Vector2 previewPosition = {.x = matrixPosition_.x + MATRIX_WIDTH * BLOCK_SIZE + 50,
                               .y = matrixPosition_.y + 10};

    int panelWidth = previewCols * BLOCK_SIZE + 2 * padding;
    int panelHeight = previewRows * BLOCK_SIZE + 2 * padding + 25;
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
    int pieceWidth = (maxCol - minCol + 1) * BLOCK_SIZE;
    int pieceHeight = (maxRow - minRow + 1) * BLOCK_SIZE;

    Vector2 pieceTopLeft = {
      .x = previewPosition.x + (float)padding + ((float)(previewCols * BLOCK_SIZE - pieceWidth) / 2),
      .y =
        previewPosition.y + (float)padding + ((float)(previewRows * BLOCK_SIZE - pieceHeight) / 2),
    };

    Vector2 localMatrix = {
      .x = pieceTopLeft.x - (float)(minCol * BLOCK_SIZE),
      .y = pieceTopLeft.y + 25 - (float)(minRow * BLOCK_SIZE),
    };

    Position zeroPosition = {};
    drawTetromino(previewTetromino, 0, zeroPosition, localMatrix);
}

void drawScore(uint32_t score, unsigned level, uint32_t linesCleared)
{
    Vector2 scorePosition = {.x = matrixPosition_.x + MATRIX_WIDTH * BLOCK_SIZE + 50,
                             .y = matrixPosition_.y + MATRIX_HEIGHT * BLOCK_SIZE - 150};
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