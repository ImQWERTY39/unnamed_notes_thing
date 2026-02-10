#ifndef PATH_H
#define PATH_H

#include "stdint.h"

#define MAX_PATH_LENGTH 256
#define OK 0
#define EXCEEDS_MAX_LENGTH 1

typedef char Path[MAX_PATH_LENGTH];

uint8_t add_to_path(Path* src, Path* sub_dir);

#endif
