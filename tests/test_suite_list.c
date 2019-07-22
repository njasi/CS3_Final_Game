#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "test_util.h"
#include "list.h"

void free_int(void *i) {
  free((int *) i);
}

List *make_list() {
  int i;
  int *data;
  List *list = list_init(1, free_int);
  for (i = 0; i < 10; i++) {
    data = (int *) malloc(sizeof(int));
    assert(data != NULL);
    *data = i + 1;
    list_add(list, data);
  }
  return list;
}

void test_get() {
  int i;
  List *list = make_list();
  for (i = 0; i< list_size(list); i++) {
    assert(*((int *)list_get(list, i))== i + 1);
  }
  list_free(list);
}

void test_add() {
  int i;
  List *list = make_list();
  for (i = 0; i < 10; i++) {
    assert(*((int *) list_get(list, i)) == i + 1);
  }
  list_free(list);
}

void test_size() {
  List *list = make_list();
  assert(list_size(list) == 10);
  list_free(list);
}

void test_remove() {
  int i, j;
  List *list = make_list();
  for (i = 1; i < 11; i++) {
    list->freer(list_remove(list, list_size(list) - 1));
    for (j = 1; j < 11 - i; j++) {
      assert(*((int *) list_get(list, j - 1)) == j);
    }
  }
  list_free(list);
}

int main(int argc, char *argv[]) {
  bool all_tests = argc == 1;
  char testname[100];
  if (!all_tests) {
    read_testname(argv[1], testname, sizeof(testname));
  }
  DO_TEST(test_get);
  DO_TEST(test_add);
  DO_TEST(test_size);
  DO_TEST(test_remove);
  puts("list_test PASS");
  return 0;
}
