#include "polygon.h"

double polygon_area(List *polygon) {
    size_t i;
    Vector a;
    Vector b;
    double area = 0.0;
    for (i = 0; i < list_size(polygon) - 1; i++) {
      a = *((Vector *) list_get(polygon, i));
      b = *((Vector *) list_get(polygon, i+1));
      area += vec_cross(a,b);
    }
    area += vec_cross(*((Vector *) list_get(polygon, i)),
                      *((Vector *) list_get(polygon, 0)));
    return area / 2;
}

Vector polygon_centroid(List *polygon) {
    size_t i;
    Vector t, s;
    double x = 0.0, y = 0.0;
    double area = polygon_area(polygon);
    for (i = 0; i < list_size(polygon) - 1; i++) {
        t = *((Vector *) list_get(polygon, i));
        s = *((Vector *) list_get(polygon, i+1));
        x += (t.x + s.x) * vec_cross(t, s);
        y += (t.y + s.y) * vec_cross(t, s);
    }
    t = *((Vector *) list_get(polygon, i));
    s = *((Vector *) list_get(polygon, 0));
    x += (t.x + s.x) * vec_cross(t, s);
    y += (t.y + s.y) * vec_cross(t, s);
    Vector centroid = { .x = x / (6 * area), .y = y / (6 * area) };
    return centroid;
}

void polygon_translate(List *polygon, Vector translation) {
    int i = 0;
    Vector v;
    Vector *temp;
    for (; i < (int) list_size(polygon); i++) {
      v = vec_add(*((Vector *) list_get(polygon, i)), translation);
      temp = (Vector *) list_get(polygon, i);
      temp->x = v.x;
      temp->y = v.y;
    }
}

void polygon_rotate(List *polygon, double angle, Vector point) {
    int i = 0;
    Vector v;
    Vector *temp;
    polygon_translate(polygon, vec_negate(point));
    for (; i < (int) list_size(polygon); i++) {
      v = vec_rotate(*((Vector *) list_get(polygon, i)), angle);
      temp = (Vector *) list_get(polygon, i);
      temp->x = v.x;
      temp->y = v.y;
    }
    polygon_translate(polygon, point);
}


List *polygon_ball(double radius, size_t num_point) {
  int i;
  Vector to_add;
  Vector *temp;
  List *points = list_init(num_point, vec_free);
  double ang_i = (2 * M_PI) / (num_point);
  for (i = 0; i < num_point; i++) {
    to_add = (Vector) { .x = radius, .y = 0.0 };
    to_add = vec_rotate(to_add, ang_i * i);
    temp = (Vector *) malloc(sizeof(Vector));
    assert(temp != NULL);
    temp->x = to_add.x, temp->y = to_add.y;
    list_add(points, (void *) temp);
  }
  return points;
}

List *polygon_rect(Vector a, Vector b){
  List *rect =list_init(4, vec_free);
  Vector *one = malloc(sizeof(Vector));
  Vector *two = malloc(sizeof(Vector));
  Vector *thr = malloc(sizeof(Vector));
  Vector *fou = malloc(sizeof(Vector));

  one->x = a.x;
  one->y = a.y;
  two->x = a.x;
  two->y = b.y;
  thr->x = b.x;
  thr->y = b.y;
  fou->x = b.x;
  fou->y = a.y;

  list_add(rect, (void *)(one));
  list_add(rect, (void *)(two));
  list_add(rect, (void *)(thr));
  list_add(rect, (void *)(fou));

  return rect;
}
