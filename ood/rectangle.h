#ifndef RECTANGLE_H
#define RECTANGLE_H
#include "shape.h"
typedef struct {
    struct ShapeVtbl const * vptr;
    Shape s;
    int width;
    int height;
}Rectangle;

void Rectangle_ctor(Rectangle * const rec, int x, int y, int width, int height);


#endif