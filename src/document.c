#include <stdint.h>
#include <string.h>

#include "../include/document.h"

static uint64_t point_as_key(Point p) {
    return ((uint64_t)(uint32_t)p.x << 32) | (uint32_t)p.y;
}

static uint64_t hash_point(uint64_t x) {
    x = (x ^ (x >> 30)) * 0xbf58476d1ce4e5b9;
    x = (x ^ (x >> 27)) * 0x94d049bb133111eb;
    x = x ^ (x >> 31);
    return x;
}

Tile* get_tile(Document* doc, Point tile_coord, uint8_t flags) {
    uint64_t key = point_as_key(tile_coord);
    if (key == doc->last_key) {
        return doc->last_tile;
    }

    uint8_t index = hash_point(key) % HASHTABLE_CAPACITY;
    uint8_t cur_index = index;

    do {
        if (!(doc->occupied[cur_index / 64] & MSB_SHIFT(cur_index % 64))) {
            if (flags & CREATE_MISSING) {
                // if (doc->length >= HASHTABLE_MAX_LIMIT)
                // flush_table(doc);

                doc->tiles[cur_index].key = key;
                doc->length += 1;
                doc->occupied[cur_index / 64] |= MSB_SHIFT(cur_index % 64);

                memset(doc->tiles[cur_index].map, 0, sizeof(BitMap));

                doc->last_key = key;
                doc->last_tile = &doc->tiles[cur_index];

                return doc->last_tile;
            }

            doc->last_key = key;
            doc->last_tile = NULL;
            return NULL;
        }

        if (doc->tiles[cur_index].key == key) {
            doc->last_key = key;
            doc->last_tile = &doc->tiles[cur_index];

            return doc->last_tile;
        }

        cur_index = (cur_index + 1) % HASHTABLE_CAPACITY;
    } while (cur_index != index);

    return NULL;
}
