#include <raylib.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct
{
    int x;
    int y;
} Position;

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

const Tetromino O = {
  .name = 'O',
  .definitions = {0b0000011001100000, 0b0000011001100000, 0b0000011001100000, 0b0000011001100000},
  .color = YELLOW
};
const Tetromino I = {
  .name = 'I',
  .definitions = {0b0000111100000000, 0b0010001000100010, 0b0000000011110000, 0b0100010001000100},
  .color = MAGENTA
};
const Tetromino J = {
  .name = 'J',
  .definitions = {0b1000111000000000, 0b0110010001000000, 0b0000111000100000, 0b0100010011000000},
  .color = BLUE
};
const Tetromino L = {
  .name = 'L',
  .definitions = {0b0010111000000000, 0b0100010001100000, 0b0000111010000000, 0b1100010001000000},
  .color = ORANGE
};
const Tetromino S = {
  .name = 'S',
  .definitions = {0b0110110000000000, 0b0100011000100000, 0b0000011011000000, 0b1000110001000000},
  .color = GREEN
};
const Tetromino Z = {
  .name = 'Z',
  .definitions = {0b1100011000000000, 0b0010011001000000, 0b0000110001100000, 0b0100110010000000},
  .color = RED
};
const Tetromino T = {
  .name = 'T',
  .definitions = {0b0100111000000000, 0b0100011001000000, 0b0000111001000000, 0b0100110001000000},
  .color = PURPLE
};

#define BLOCK_SIZE 20;
#define MATRIX_WIDTH 10
#define MATRIX_HEIGHT 20

const Tetromino blockTypes[] = {O, Z, J, L, S, T, I};
const uint32_t basePoints[4] = {40, 100, 300, 1200};
