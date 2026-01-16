#include "../include/tiles.h"

inline int32_t floor_div(int32_t a) {
    int32_t q = a / TILE_SIZE;

    if (a < 0 && a % TILE_SIZE != 0)
        q--;

    return q;
}

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

void serialize_tile(FILE* file, Tile* tile) {
    uint64_t written_rows[4] = {0};

    for (size_t r = 0; r < TILE_SIZE; r++) {
        if (tile->map[r][0] || tile->map[r][1] || tile->map[r][2] || tile->map[r][3]) {
            written_rows[r / 64] |= MSB_SHIFT(r % 64);
        }
    }

    fwrite(written_rows, sizeof(uint64_t), 4, file);

    for (size_t r = 0; r < TILE_SIZE; r++) {
        if (written_rows[r / 64] & MSB_SHIFT(r % 64)) {
            fwrite(tile->map[r], sizeof(uint64_t), 4, file);
        }
    }
}

void deserialize_tile(FILE* file, Tile* tile) {
    uint64_t written_rows[4] = {0};

    uint32_t bytes_read = fread(written_rows, sizeof(uint64_t), 4, file);
    if (bytes_read != 4) {
        memset(tile->map, 0, sizeof(BitMap));
        return;
    }

    for (size_t r = 0; r < TILE_SIZE; r++) {
        if (written_rows[r / 64] & MSB_SHIFT(r % 64)) {
            fread(tile->map[r], sizeof(uint64_t), 4, file);
        } else {
            memset(
                tile->map[r], 0, sizeof(uint64_t) * 4
            ); // will have to change for tablet PLEASE remember
        }
    }
}
