#include "../include/document.h"
#include "../include/fileio.h"
#include "../include/tiles.h"
#include <stdio.h>

const char* BASE64_ENCODING = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz+-";
const uint32_t BASE64_DECODING[128] = {
    ['0'] = 0,  ['1'] = 1,  ['2'] = 2,  ['3'] = 3,  ['4'] = 4,  ['5'] = 5,  ['6'] = 6,  ['7'] = 7,
    ['8'] = 8,  ['9'] = 9,  ['A'] = 10, ['B'] = 11, ['C'] = 12, ['D'] = 13, ['E'] = 14, ['F'] = 15,
    ['G'] = 16, ['H'] = 17, ['I'] = 18, ['J'] = 19, ['K'] = 20, ['L'] = 21, ['M'] = 22, ['N'] = 23,
    ['O'] = 24, ['P'] = 25, ['Q'] = 26, ['R'] = 27, ['S'] = 28, ['T'] = 29, ['U'] = 30, ['V'] = 31,
    ['W'] = 32, ['X'] = 33, ['Y'] = 34, ['Z'] = 35, ['a'] = 36, ['b'] = 37, ['c'] = 38, ['d'] = 39,
    ['e'] = 40, ['f'] = 41, ['g'] = 42, ['h'] = 43, ['i'] = 44, ['j'] = 45, ['k'] = 46, ['l'] = 47,
    ['m'] = 48, ['n'] = 49, ['o'] = 50, ['p'] = 51, ['q'] = 52, ['r'] = 53, ['s'] = 54, ['t'] = 55,
    ['u'] = 56, ['v'] = 57, ['w'] = 58, ['x'] = 59, ['y'] = 60, ['z'] = 61, ['+'] = 62, ['-'] = 63
};

void to_filename(char* name, uint64_t key) {
    // y stored in lower 4 bytes (only 3 used)
    name[0] = BASE64_ENCODING[key & 63];
    name[1] = BASE64_ENCODING[(key >> 6) & 63];
    name[2] = BASE64_ENCODING[(key >> 12) & 63];
    name[3] = BASE64_ENCODING[(key >> 18) & 63];

    // x stored in upper 4 bytes (only 3 used)
    name[4] = BASE64_ENCODING[(key >> 32) & 63];
    name[5] = BASE64_ENCODING[(key >> 38) & 63];
    name[6] = BASE64_ENCODING[(key >> 44) & 63];
    name[7] = BASE64_ENCODING[(key >> 50) & 63];

    name[8] = '.';
    name[9] = 'b';
    name[10] = 'i';
    name[11] = 'n';
    name[12] = '\0';
}

Point from_filename(const char* name) {
    int32_t x = BASE64_DECODING[(uint8_t)name[4]] | BASE64_DECODING[(uint8_t)name[5]] << 6 |
        BASE64_DECODING[(uint8_t)name[6]] << 12 | BASE64_DECODING[(uint8_t)name[7]] << 18;
    if (x & 0x800000) {
        x |= 0xFF000000;
    }
    int32_t y =
        (int32_t)(BASE64_DECODING[(uint8_t)name[0]] | BASE64_DECODING[(uint8_t)name[1]] << 6 |
                  BASE64_DECODING[(uint8_t)name[2]] << 12 |
                  BASE64_DECODING[(uint8_t)name[3]] << 18);
    if (y & 0x800000) {
        y |= 0xFF000000;
    }

    return (Point){x, y};
}

void serialize_tile(Tile* tile) {
    char file_name[13] = {0};
    to_filename(file_name, tile->key);
    FILE* file = fopen(file_name, "wb");

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

    fclose(file);
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

void load_file(Document* document, Point locality) {
    locality = tile_coords(locality);
    if (document->length >= HASHTABLE_MAX_LIMIT) {
        flush_document(document);
    }

    for (int32_t x = -3; x < 11; x++) {
        for (int32_t y = -3; y < 8; y++) {
            Point t_coord = (Point){locality.x + x, locality.y + y};
            uint64_t key = point_as_key(t_coord);

            char name[13] = {0};
            to_filename(name, key);

            FILE* f = fopen(name, "rb");
            if (f == NULL) {
                continue;
            }

            Tile* t = get_tile(document, t_coord, CREATE_MISSING);
            deserialize_tile(f, t);
            fclose(f);
        }
    }
}

void flush_document(Document* doc) {
    for (size_t i = 0; i < HASHTABLE_CAPACITY; i++) {
        if (doc->occupied[i / 64] & MSB_SHIFT(i % 64)) {
            serialize_tile(&doc->tiles[i]);
        }
    }

    doc->length = 0;
    memset(doc->occupied, 0, sizeof(doc->occupied));
    memset(doc->tiles, 0, sizeof(doc->tiles));
    doc->last_key = 0xFFFFFFFFFFFFFFFF;
    doc->last_tile = NULL;
}
