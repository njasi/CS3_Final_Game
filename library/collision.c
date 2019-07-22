#include "collision.h"
#include "body.h"

bool find_collision(List *shape1, List *shape2) {
    edges1 = get_edges(shape1);
    for (i = 0; i < list_size(shape2); i++) {
      inside = true;
      s = *((Vector *) list_get(shape2, i));
      for (j = 0; j < list_size(edges1); j++) {
        e = *((Vector *) list_get(edges1, j));
        v = *((Vector *) list_get(shape1, j));
        p = vec_subtract(s, v);
        if (vec_cross(e, p) < 0) {
          inside = false;
        }
      }
      if (inside) {
        list_free(edges1);
        return true;
      }
    }
    list_free(edges1);
    return false;
}