#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "list.h"

#define GROWTH_FACTOR 2

List *list_init(size_t initial_size, FreeFunc freer) {
  List *list = (List *) malloc(sizeof(List));
  assert(list != NULL);
  list->data = (void **) malloc((int) initial_size * sizeof(void *));
  assert(list->data != NULL);
  list->freer = freer;
  list->size = 0;
  list->capacity = initial_size;
  return list;
}

void list_free(List *list) {
  int i;
  if (list->freer != NULL) {
    for (i = 0; i < (int) list->size; i++) {
      list->freer(list->data[i]);
    }
  }
  free(list->data);
  free(list);
}

size_t list_size(List *list) {
  return list->size;
}

void *list_get(List *list, size_t index) {
  assert(index >= 0 && index < list->size);
  return list->data[(int) index];
}

void *list_remove(List *list, size_t index) {
  assert(index >= 0 && index < list->size);
  int i = (int) index;
  void *remove = list->data[i];
  for (; i < (int) list->size - 1; i++) {
    list->data[i] = list->data[i+1];
  }
  list->size--;
  return remove;
}

void list_add(List *list, void *value) {
  if (list->size == list->capacity) {
    list->capacity *= GROWTH_FACTOR;
    list->data = (void **) realloc(list->data, (int) list->capacity *
                                   sizeof(void *));
    assert(list->data != NULL);
  }
  list->data[list->size] = value;
  list->size++;
}

List *list_append(List *list1, List *list2) {
  size_t i;
  assert(list1->freer == list2->freer);
  List *list3 = list_init(list_size(list1) + list_size(list2), list1->freer);
  for (i = 0; i < list_size(list1); i++) {
    list_add(list3, list_get(list1, i));
  }
  for (i = 0; i < list_size(list2); i++) {
    list_add(list3, list_get(list2, i));
  }
  return list3;
}
