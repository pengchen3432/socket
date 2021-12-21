#ifndef SHAPE_H
#define SHAPE_H
struct ShapeVtbl;
typedef struct {
    struct ShapeVtbl const *vptr;
    int x;
    int y;
}Shape;
struct ShapeVtbl {
    int (*area) (Shape * const shape);
    void (*draw) (Shape * const Shape);
};

void Shape_ctor(Shape *const me, int x, int y);
void Shape_move(Shape *const me, int dx, int dy);
int Shape_get_x(Shape *const me);
int Shape_get_y(Shape *const me);

static inline int Shape_area(Shape * const me) {
    return me->vptr->area(me);
}
static inline void Shape_draw(Shape * const me) {
    return me->vptr->draw(me);
}
#endif