#ifndef TILE_C
#define TILE_C

#include "../include/geometry.h"

#define TILE_SIZE 256
#define MSB_SHIFT(x) (0x8000000000000000 >> (x))

typedef uint64_t BitMap[TILE_SIZE][TILE_SIZE / 64];

typedef struct {
    uint64_t key;
    BitMap map;
} Tile;

inline int32_t floor_div(int32_t a);
Point tile_coords(Point global_coords);
void set(Tile* tile, uint8_t x, uint8_t y);
void clear(Tile* tile, uint8_t x, uint8_t y);

#endif
