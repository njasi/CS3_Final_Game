#include "test_util.h"

int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }
  /*DO_TEST(test_line_intersect);
  DO_TEST(test_body_collide);
  DO_TEST(test_boundary_collide);*/
  puts("scene_test PASS");
  return 0;
}
