#ifndef DOCUMENT_C
#define DOCUMENT_C

#include "geometry.h"
#include "tiles.h"
#include <stdlib.h>

#define HASHTABLE_CAPACITY 256
#define HASHTABLE_MAX_LIMIT 180

#define IGNORE 0b00000000
#define CREATE_MISSING 0b00000001

typedef struct {
    size_t length;
    uint64_t occupied[4];
    uint64_t last_key;
    Tile* last_tile;
    Tile tiles[HASHTABLE_CAPACITY];
} Document;

Tile* get_tile(Document* doc, Point tile_coord, uint8_t flags);

#endif
