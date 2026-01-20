#include "../include/tiles.h"

Point tile_coords(Point global_coords) {
    global_coords.x = floor_div(global_coords.x);
    global_coords.y = floor_div(global_coords.y);
    return global_coords;
}

void set(Tile* tile, uint8_t x, uint8_t y) {
    tile->map[y][x / 64] |= MSB_SHIFT(x % 64);
}

void clear(Tile* tile, uint8_t x, uint8_t y) {
    tile->map[y][x / 64] &= ~MSB_SHIFT(x % 64);
}
