#ifndef DOCUMENT_C
#define DOCUMENT_C

#include "geometry.h"
#include "tiles.h"
#include <stdlib.h>

#define HASHTABLE_CAPACITY 256
#define HASHTABLE_MAX_LIMIT 180

const uint8_t IGNORE = 0b00000000;
const uint8_t CREATE_MISSING = 0b00000001;

typedef struct {
    Tile tiles[HASHTABLE_CAPACITY];
    size_t length;
    uint64_t occupied[4];
    uint64_t last_key;
    Tile* last_tile;
} Document;

Tile* get_tile(Document* doc, Point tile_coord, uint8_t flags);

// void flush_document(Document* doc);

#endif
