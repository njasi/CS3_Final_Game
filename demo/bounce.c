#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "list.h"
#include "color.h"
#include <stdio.h>

#define POINTS 5
#define SIZE 10
#define SIZE_RATIO 0.5
#define SPEED 100
#define ANGULAR_SPEED 1

const Vector min = (Vector) { .x = -200, .y = -100};
const Vector max = (Vector) { .x = 200, .y = 100 };
const Vector start = (Vector) {50, 50};

void vector_free(void *v);
void update(List *polygon);
List *make_star(double radius, double size_ratio, size_t
num_point);

Vector dir = (Vector) {1, 1};

int main(void) {
    sdl_init(min, max);
    List *star = make_star(SIZE, SIZE_RATIO, POINTS);
    while (!sdl_is_done()) {
          sdl_clear();
          update(star);
          sdl_draw_polygon(star, (RGBColor) {.5, .5, .5});
          sdl_show();
    }
    list_free(star);
    return 0;
}

void vector_free(void *v) {
  free((Vector *) v);
}

void update(List *polygon) {
  double time = time_since_last_tick();
  double distance = SPEED * time;
  double rotation = ANGULAR_SPEED * time;
  Vector centroid = polygon_centroid(polygon);
  if (dir.x < 0) {
    if (centroid.x - SIZE - distance <= min.x) {
      dir.x *= -1;
    }
  }
  else {
    if (centroid.x + SIZE + distance >= max.x) {
      dir.x *= -1;
    }
  }
  if (dir.y < 0) {
    if (centroid.y - SIZE - distance <= min.y) {
      dir.y *= -1;
    }
  }
  else {
    if (centroid.y + SIZE + distance >= max.y) {
      dir.y *= -1;
    }
  }
  Vector translation = (Vector) {dir.x * distance, dir.y * distance};
  polygon_translate(polygon, translation);
  polygon_rotate(polygon, rotation, centroid);
}

/*
 *  Create a star with n points around (0,0)
 *  given outer radius, ratio of radiuses sizes, number of points,
 */
 List *make_star(double radius, double size_ratio, size_t num_point) {
   int i;
   Vector to_add;
   Vector *temp;
   List *points = list_init(num_point * 2, vec_free);
   double ang_i = (2 * M_PI) / (2 * num_point);
   for (i = 0; i < num_point * 2; i++) {
     to_add = (Vector) { .x = radius, .y = 0.0 };
     if (!(i & 1)) {
       to_add = vec_multiply(size_ratio, to_add);
     }
     to_add = vec_rotate(to_add, ang_i * i);
     temp = (Vector *) malloc(sizeof(Vector));
     assert(temp != NULL);
     temp->x = to_add.x, temp->y = to_add.y;
     list_add(points, (void *) temp);
   }
   return points;
 }
