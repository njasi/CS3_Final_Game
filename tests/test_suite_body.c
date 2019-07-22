#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "test_util.h"
#include "vector.h"
#include "body.h"

List *make_square() {
  List *sq = list_init(4, vec_free);
  Vector *v1 = (Vector *) malloc(sizeof(Vector));
  v1->x = 1, v1->y = 1;
  Vector *v2 = (Vector *) malloc(sizeof(Vector));
  v2->x = -1, v2->y = 1;
  Vector *v3 = (Vector *) malloc(sizeof(Vector));
  v3->x = -1, v3->y = -1;
  Vector *v4 = (Vector *) malloc(sizeof(Vector));
  v4->x = 1, v4->y = -1;
  list_add(sq, v1);
  list_add(sq, v2);
  list_add(sq, v3);
  list_add(sq, v4);
  return sq;
}

void test_line_intersect() {
  line l1 = {(Vector){0, 5}, (Vector){5, 0}};
  line l2 = {(Vector){0, 0}, (Vector){5, 5}};
  line l3 = {(Vector){0, 0}, (Vector){5, 1}};
  line l4 = {(Vector){0, 5}, (Vector){5, 2}};
  assert(line_intersect(l1, l2) == true);
  assert(line_intersect(l3, l4) == false);
}

void test_boundary_collide() {
  size_t i;
  List *square = make_square();
  double bt = 0, bf = 2;
  for (i = 0; i < 4; i++) {
    assert(boundary_collide(square, bt, i) == true);
  }
  for (i = 0; i < 4; i++) {
    assert(boundary_collide(square,
                            (((i + 1) % 3) == 1 ? -1 : 1) * bf, i) == false);
  }
  list_free(square);
}

int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }
  DO_TEST(test_line_intersect);
  DO_TEST(test_boundary_collide);
  puts("body_test PASS");
  return 0;
}
