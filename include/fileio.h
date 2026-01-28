#ifndef FILEIO_C
#define FILEIO_C

#include "document.h"

#define VIEWPORT_CACHE_SIZE 64

void to_filename(char* name, uint64_t key);
Point from_filename(const char* name);

void serialize_tile(Tile* tile);
void deserialize_tile(FILE* file, Tile* tile);

void load_file(Document* doc);
void flush_document(Document* doc);

/*
have load_file (or new func) take coordinates of the top left corner
to load tiles around that coord...

count for possibility of overflow (how?), flush before loading if overflow
*/

#endif
