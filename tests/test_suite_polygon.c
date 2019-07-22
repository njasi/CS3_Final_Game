#include "vec_list.h"
#include "vector.h"
#include "polygon.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdbool.h>
#include <stdlib.h>

// Make square at (+/-1, +/-1)

List *make_square() {
    List *sq = list_init(4, vec_free);
    Vector *a = malloc(sizeof(Vector));
    a->x = 1;
    a->y = 1;
    Vector *b = malloc(sizeof(Vector));
    b->x = -1;
    b->y = 1;
    Vector *c = malloc(sizeof(Vector));
    c->x = -1;
    c->y = -1;
    Vector *d = malloc(sizeof(Vector));
    d->x = 1;
    d->y = -1;
    list_add(sq, (void *)a);
    list_add(sq, (void *)b);
    list_add(sq, (void *)c);
    list_add(sq, (void *)d);
    return sq;
}

void test_square_area_centroid() {
    List *sq = make_square();
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), VEC_ZERO));
    list_free(sq);
}

void test_square_translate() {
    List *sq = make_square();
    polygon_translate(sq, (Vector){2, 3});
    assert(vec_equal(*(Vector *)list_get(sq, 0), (Vector){3, 4}));
    assert(vec_equal(*(Vector *)list_get(sq, 1), (Vector){1, 4}));
    assert(vec_equal(*(Vector *)list_get(sq, 2), (Vector){1, 2}));
    assert(vec_equal(*(Vector *)list_get(sq, 3), (Vector){3, 2}));
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), (Vector){2, 3}));
    list_free(sq);
}

void test_square_rotate() {
    List *sq = make_square();
    polygon_rotate(sq, 0.25 * M_PI, VEC_ZERO);
    assert(vec_isclose(*(Vector *)list_get(sq, 0), (Vector){0, sqrt(2)}));
    assert(vec_isclose(*(Vector *)list_get(sq, 1), (Vector){-sqrt(2), 0}));
    assert(vec_isclose(*(Vector *)list_get(sq, 2), (Vector){0, -sqrt(2)}));
    assert(vec_isclose(*(Vector *)list_get(sq, 3), (Vector){sqrt(2), 0}));
    assert(isclose(polygon_area(sq), 4));
    assert(vec_isclose(polygon_centroid(sq), VEC_ZERO));
    list_free(sq);
}

// Make 3-4-5 triangle
List *make_triangle() {
    List *tri = list_init(3, vec_free);
    Vector *a = malloc(sizeof(Vector));
    a->x = 0;
    a->y = 0;
    Vector *b = malloc(sizeof(Vector));
    b->x = 4;
    b->y = 0;
    Vector *c = malloc(sizeof(Vector));
    c->x = 4;
    c->y = 3;
    list_add(tri, (void *)a);
    list_add(tri, (void *)b);
    list_add(tri, (void *)c);
    return tri;
}

void test_triangle_area_centroid() {
    List *tri = make_triangle();
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){8.0 / 3.0, 1}));
    list_free(tri);
}

void test_triangle_translate() {
    List *tri = make_triangle();
    polygon_translate(tri, (Vector){-4, -3});
    assert(vec_equal(*(Vector *)list_get(tri, 0), (Vector){-4, -3}));
    assert(vec_equal(*(Vector *)list_get(tri, 1), (Vector){0,  -3}));
    assert(vec_equal(*(Vector *)list_get(tri, 2), (Vector){0,  0}));
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){-4.0 / 3.0, -2}));
    list_free(tri);
}

void test_triangle_rotate() {
    List *tri = make_triangle();

    // Rotate -acos(4/5) degrees around (4,3)
    polygon_rotate(tri, 2 * M_PI - acos(4.0 / 5.0), (Vector){4, 3});
    assert(vec_isclose(*(Vector *)list_get(tri, 0), (Vector){-1,  3}));
    assert(vec_isclose(*(Vector *)list_get(tri, 1), (Vector){2.2, 0.6}));
    assert(vec_isclose(*(Vector *)list_get(tri, 2), (Vector){4,   3}));
    assert(isclose(polygon_area(tri), 6));
    assert(vec_isclose(polygon_centroid(tri), (Vector){26.0 / 15.0, 2.2}));

    list_free(tri);
}

#define CIRC_NPOINTS 1000000
#define CIRC_AREA (CIRC_NPOINTS * sin(2 * M_PI / CIRC_NPOINTS) / 2)

// Circle with many points (stress test)
List *make_big_circ() {
    Vector to_add;
    Vector *temp;
    List *c = list_init(CIRC_NPOINTS, vec_free);
    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        to_add = (Vector){cos(angle), sin(angle)};
        temp = malloc(sizeof(Vector));
        temp->x = to_add.x, temp->y = to_add.y;
        list_add(c, (void *)temp);
    }
    return c;
}

void test_circ_area_centroid() {
    List *c = make_big_circ();
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), VEC_ZERO));
    list_free(c);
}

void test_circ_translate() {
    List *c = make_big_circ();
    Vector translation = {.x = 100, .y = 200};
    polygon_translate(c, translation);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        assert(vec_isclose(
            *(Vector *)list_get(c, i),
            vec_add(translation, (Vector){cos(angle), sin(angle)})
        ));
    }
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), translation));

    list_free(c);
}

void test_circ_rotate() {
    // Rotate about the origin at an unusual angle
    const double rot_angle = 0.5;

    List *c = make_big_circ();
    polygon_rotate(c, rot_angle, VEC_ZERO);

    for (size_t i = 0; i < CIRC_NPOINTS; i++) {
        double angle = 2 * M_PI * i / CIRC_NPOINTS;
        assert(vec_isclose(
            *(Vector *)list_get(c, i),
            (Vector){cos(angle + rot_angle), sin(angle + rot_angle)})
        );
    }
    assert(isclose(polygon_area(c), CIRC_AREA));
    assert(vec_isclose(polygon_centroid(c), VEC_ZERO));

    list_free(c);
}

// Weird nonconvex polygon
List *make_weird() {
    List *w = list_init(5, vec_free);

    Vector *a = malloc(sizeof(Vector));
    a->x = 0;
    a->y = 0;
    Vector *b = malloc(sizeof(Vector));
    b->x = 4;
    b->y = 1;
    Vector *c = malloc(sizeof(Vector));
    c->x = -2;
    c->y = 1;
    Vector *d = malloc(sizeof(Vector));
    d->x = -5;
    d->y = 5;
    Vector *e = malloc(sizeof(Vector));
    e->x = -1;
    e->y = -8;

    list_add(w, (void *) a);
    list_add(w, (void *) b);
    list_add(w, (void *) c);
    list_add(w, (void *) d);
    list_add(w, (void *) e);

    return w;
}

void test_weird_area_centroid() {
    List *w = make_weird();
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){-223.0 / 138.0, -51.0 / 46.0}));
    list_free(w);
}

void test_weird_translate() {
    List *w = make_weird();
    polygon_translate(w, (Vector){-10, -20});

    assert(vec_isclose(*(Vector *)list_get(w, 0), (Vector){-10, -20}));
    assert(vec_isclose(*(Vector *)list_get(w, 1), (Vector){-6,  -19}));
    assert(vec_isclose(*(Vector *)list_get(w, 2), (Vector){-12, -19}));
    assert(vec_isclose(*(Vector *)list_get(w, 3), (Vector){-15, -15}));
    assert(vec_isclose(*(Vector *)list_get(w, 4), (Vector){-11, -28}));
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){-1603.0 / 138.0, -971.0 / 46.0}));

    list_free(w);
}

void test_weird_rotate() {
    List *w = make_weird();
    // Rotate 90 degrees around (0, 2)
    polygon_rotate(w, M_PI / 2, (Vector){0, 2});

    assert(vec_isclose(*(Vector *)list_get(w, 0), (Vector){2,  2}));
    assert(vec_isclose(*(Vector *)list_get(w, 1), (Vector){1,  6}));
    assert(vec_isclose(*(Vector *)list_get(w, 2), (Vector){1,  0}));
    assert(vec_isclose(*(Vector *)list_get(w, 3), (Vector){-3, -3}));
    assert(vec_isclose(*(Vector *)list_get(w, 4), (Vector){10, 1}));
    assert(isclose(polygon_area(w), 23));
    assert(vec_isclose(polygon_centroid(w), (Vector){143.0 / 46.0, 53.0 / 138.0}));

    list_free(w);
}

int main(int argc, char *argv[]) {
    // Run all tests? True if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_square_area_centroid);
    DO_TEST(test_square_translate);
    DO_TEST(test_square_rotate);
    DO_TEST(test_triangle_area_centroid);
    DO_TEST(test_triangle_translate);
    DO_TEST(test_triangle_rotate);
    DO_TEST(test_circ_area_centroid);
    DO_TEST(test_circ_translate);
    DO_TEST(test_circ_rotate);
    DO_TEST(test_weird_area_centroid);
    DO_TEST(test_weird_translate);
    DO_TEST(test_weird_rotate);

    puts("polygon_test PASS");

    return 0;
}
