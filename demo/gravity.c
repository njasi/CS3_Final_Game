#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "color.h"
#include <stdio.h>

#define SIZE 4             // star outer circle radius
#define SIZE_RATIO 0.5     // star inner circle r : outer circle r
#define SPEED 15           // scrolling speed
#define ANGULAR_SPEED 0.5  // star rotating speed
#define GRAVITY -200       // g constant of our demo
#define STAR_RATE 3        // wait time in seconds between each star creation
#define WINDOW_WIDTH 100
#define WINDOW_HEIGHT 50

void update(Body *polygon, double time);
List *make_star(double radius, double size_ratio, size_t num_point);
List *make_line(Vector a, Vector b);

const Vector min = (Vector) { .x = 0, .y = 0 };
const Vector max = (Vector) { .x = WINDOW_WIDTH, .y = WINDOW_HEIGHT };
const Vector start = (Vector) { .x = 0, .y = WINDOW_HEIGHT };

float time_elapsed = STAR_RATE;
List *bottom;
List *reaper;
/*
 * reaper is the line that the bodies will come into contact with offscreen to
 * be deleted
 */

int main(void) {

  // init all vars before loop
  bottom = make_line(min, (Vector) { .x = max.x, .y = 0 });
  reaper = make_line(max, (Vector) { .x = max.x + (SIZE * 2), .y = 0 });

  size_t i;
  float r = 0, g = 0, b = 0;
  int num_points = 1;
  double time = 0.0;
  List *star;
  Body *temp;
  RGBColor color;

  List *list = list_init(1, body_free);

  sdl_init(min, max); // window init

  while (!sdl_is_done()) {
    if(time_elapsed >= STAR_RATE) { // generates the newest star
      time_elapsed = 0;
      //printf("Creating the %d-pointed star.\n",num_points);
      num_points++;
      r = ((float) rand()) / RAND_MAX;
      g = ((float) rand()) / RAND_MAX;
      b = ((float) rand()) / RAND_MAX;
      color = (RGBColor) {r, g, b};
      star = make_star(SIZE, SIZE_RATIO, num_points);
      polygon_translate(star, start);
      // add in new body to the list  the new body is listed below
      temp = body_init(star, 0, color);
      body_set_velocity(temp, (Vector) { .x = SPEED, .y = 0 });
      temp->elasticity = 0.95;
      temp->angular_speed = ANGULAR_SPEED;
      list_add(list, temp);
    }

    sdl_clear();

    time = time_since_last_tick();
    time_elapsed += time;
    for (i = 0; i < list->size; i++) {
      temp = list_get(list, i);
      update(temp, time);
      sdl_draw_polygon(temp->polygon, temp->fill_color);
    }

    for (i = 0; i < list->size; i++) {
      // remove it from the list because it has touched the reaper
      if (polygon_centroid(temp->polygon).x >= WINDOW_WIDTH + (2 * SIZE)) {
        //printf("removing off-screen star");
        list->freer(list_remove(list, list_size(list) - 1));
      }
    }

    sdl_show();
  }


  list_free(bottom);
  list_free(reaper);
  list_free(list);

  return 0;
}

/**
* Updates a body object by applying gravity and bouncing off of the bottom of
* the screen. Uses the elasticity stored in the body struct to lose height.
*
* @param obj the body that we are going to update
* @param time the time step for which to update the body
*/
void update(Body *obj, double time) {
  Vector centroid = polygon_centroid(obj->polygon);
  // collide with bottom line
  if (centroid.y <= SIZE && boundary_collide(obj->polygon, 0, 0)) {
    //printf("%zu-pointed star bounced!\n",vec_list_size(obj->polygon)/2);
    obj->velocity.y = fabs(obj->velocity.y * obj->elasticity);
  }
  obj->velocity.y += GRAVITY * time;

  double dist_x = obj->velocity.x * time, dist_y = obj->velocity.y * time;
  double rotation = obj->angular_speed * time;
  //printf("Num: %zu \tX: %f\tY: %f\n",vec_list_size(obj->polygon), centroid.x, centroid.y);

  Vector translation = (Vector) { .x = dist_x, .y = dist_y };
  polygon_translate(obj->polygon, translation);
  polygon_rotate(obj->polygon, rotation, centroid);
}

/**
 * Create a star with n points around (0,0)
 *
 * @param radius outer radius of star
 * @param size_ratio ratio of star inner circle to star outer circle
 *        (2 will invert)
 * @param num_point the number of points of the star (2 makes a rhombus)
 */
 List *make_star(double radius, double size_ratio, size_t num_point) {
   size_t i;
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

/**
 * Create a line with 2 points a and b
 *
 * @param a point a
 * @param b point b
 */
List *make_line(Vector a, Vector b) {
  List *line = list_init(2, vec_free);
  Vector *va = (Vector *) malloc(sizeof(Vector));
  Vector *vb = (Vector *) malloc(sizeof(Vector));
  assert(va != NULL && vb != NULL);
  va->x = a.x, va->y = a.y;
  vb->x = b.x, vb->y = b.y;
  list_add(line, (void *) va);
  list_add(line, (void *) vb);
  return line;
}
