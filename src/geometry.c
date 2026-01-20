#include "../include/geometry.h"

uint32_t distance_sq(Point a, Point b) {
    uint32_t delta_x = (uint32_t)(a.x - b.x);
    uint32_t delta_y = (uint32_t)(a.y - b.y);

    return delta_x * delta_x + delta_y * delta_y;
}

uint64_t point_as_key(Point p) {
    return ((uint64_t)(uint32_t)p.x << 32) | (uint32_t)p.y;
}
