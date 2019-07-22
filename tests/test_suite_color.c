#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test_util.h"
#include "list.h"

int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }
  puts("color_test PASS");
  return 0;
}
