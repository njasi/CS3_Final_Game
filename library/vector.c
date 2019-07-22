#include <stdlib.h>
#include <math.h>
#include "vector.h"

const Vector VEC_ZERO = { .x = 0.0, .y = 0.0 };

Vector vec_add(Vector v1, Vector v2) {
    Vector v3 = { .x = v1.x + v2.x, .y = v1.y + v2.y };
    return v3;
}

Vector vec_subtract(Vector v1, Vector v2) {
    Vector v3 = { .x = v1.x - v2.x, v1.y - v2.y };
    return v3;
}

Vector vec_negate(Vector v) {
    Vector vec = { .x = -1 * v.x, .y = -1 * v.y };
    return vec;
}

Vector vec_multiply(double scalar, Vector v) {
    Vector vec = { .x = scalar * v.x, .y = scalar * v.y };
    return vec;
}

double vec_dot(Vector v1, Vector v2) {
    return v1.x * v2.x + v1.y * v2.y;
}

double vec_cross(Vector v1, Vector v2) {
    return v1.x * v2.y - v1.y * v2.x;
}

Vector vec_rotate(Vector v, double angle) {
    Vector vec = { .x = v.x * cos(angle) - v.y * sin(angle),
                   .y = v.x * sin(angle) + v.y * cos(angle) };
    return vec;
}

double vec_distance(Vector a, Vector b) {
    return sqrt(pow(a.x - b.x, 2) + pow(a.y - b.y, 2));
}

Vector vec_unit(Vector v) {
    double mag = sqrt(vec_dot(v, v));
    return (Vector) { .x = v.x / mag, .y = v.y / mag };
}

void vec_free(void *vec) {
  free((Vector *) vec);
}
