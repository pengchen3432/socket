#include "shape.h"
#include <stdio.h>
#include <assert.h>

static int Shape_area_(Shape * const me);
static void Shape_draw_(Shape * const me);

static int Shape_area_(Shape * const me) {
    assert(0);
    return 0;
}
static void Shape_draw_(Shape * const me) {
    assert(0);
}
void Shape_ctor(Shape *const me, int x, int y) {
    static struct ShapeVtbl vptr = {
        &Shape_area_,
        &Shape_draw_
    };
    me->vptr = &vptr;
    me->x = x;
    me->y = y;
}
void Shape_move(Shape *const me, int dx, int dy) {
    me->x += dx;
    me->y += dy;
}
int Shape_get_x( Shape *const me) {
    return me->x;
}
int Shape_get_y(Shape *const me) {
    return me->y;
}