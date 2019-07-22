#include <stdlib.h>
#include <assert.h>
#include "vec_list.h"

VectorList *vec_list_init(size_t initial_size) {
    VectorList *vlist = malloc(sizeof(VectorList));
    assert(vlist != NULL);
    vlist->size = 0;
    vlist->capacity = initial_size;
    vlist->data = malloc(initial_size * sizeof(Vector));
    assert(vlist->data != NULL);
    return vlist;
}

void vec_list_free(VectorList *list) {
    free(list->data);
    free(list);
}

size_t vec_list_size(VectorList *list) {
    return list->size;
}

Vector vec_list_get(VectorList *list, size_t index) {
    assert(index >= 0 && index < list->size); // fails this
    return list->data[index];
}

void vec_list_set(VectorList *list, size_t index, Vector value) {
    assert(index >= 0 && index < list->size);
    list->data[index] = value;
}

void vec_list_add(VectorList *list, Vector value) {
    assert(list->size < list->capacity);
    list->size++;
    list->data[list->size-1] = value;
}

Vector vec_list_remove(VectorList *list) {
    assert(list->size > 0);
    list->size--;
    return list->data[list->size];
}
