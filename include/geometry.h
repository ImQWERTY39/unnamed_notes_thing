#ifndef GEOMETRY_C
#define GEOMETRY_C

#include <stdint.h>

typedef struct {
    int32_t x;
    int32_t y;
} Point;

uint32_t distance_sq(Point a, Point b);
uint64_t point_as_key(Point p);

#endif
