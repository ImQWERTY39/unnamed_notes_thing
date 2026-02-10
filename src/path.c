#include "../include/path.h"
#include <stdlib.h>

static size_t length(Path* dir) {
    size_t length = 0;
    while ((*dir)[length++]) {}
    return length;
}

uint8_t add_to_path(Path* src, Path* sub_dir) {
    size_t old_size = length(src);
    size_t new_size = old_size + length(sub_dir);

    if (new_size > 255) {
        return EXCEEDS_MAX_LENGTH;
    }

    uint8_t idx = 0;
    while (sub_dir[idx++]) {
        (*src)[idx + old_size] = (*sub_dir)[idx];
    }

    return OK;
}
