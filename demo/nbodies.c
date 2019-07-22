#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "color.h"
#include "forces.h"
#include <stdio.h>

#define NUM_BODIES 20
#define STAR_PTS 7
#define BIGGEST 12
#define SMALLEST 6
#define MASS 100
#define STAR_RATIO 0.5
#define ROTATION 100

const double G = 6.67408 * 10E11;

const Vector min = (Vector) { .x = -200, .y = -100};
const Vector max = (Vector) { .x = 200, .y = 100 };

int random_in_range(int max, int min);

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
  polygon_translate(points, (Vector){random_in_range(max.x, min.x),
                                     random_in_range(max.y, min.y)});
  return points;
}

List *make_stars(){
  List *stars = list_init(NUM_BODIES, body_free);
  size_t i = 0;
  for(;i < NUM_BODIES; i++){
    List *star = make_star(random_in_range(BIGGEST, SMALLEST), STAR_RATIO, STAR_PTS);
    Body *temp = body_init(star, MASS, color_rainbow(NUM_BODIES,i));
    temp->angular_speed = random_in_range(ROTATION, -1 * ROTATION);
    list_add(stars, (void *)temp);
  }
  return stars;
}

Body *make_rect(Vector max, Vector min){
  List *rect =list_init(4, vec_free);
  Vector *one = malloc(sizeof(Vector));
  Vector *two = malloc(sizeof(Vector));
  Vector *thr = malloc(sizeof(Vector));
  Vector *fou = malloc(sizeof(Vector));

  one->x = max.x;
  one->y = max.y;
  two->x = max.x;
  two->y = min.y;
  thr->x = min.x;
  thr->y = min.y;
  fou->x = min.x;
  fou->y = max.y;

  list_add(rect, (void *)(one));
  list_add(rect, (void *)(two));
  list_add(rect, (void *)(thr));
  list_add(rect, (void *)(fou));

  Body *background = body_init(rect, 0, (RGBColor){0, 0, 0});
  return background;
}


void add_gravity(Scene *s, List *stars){
  size_t i = 0;
  size_t j;
  for(; i < list_size(stars); i++){
    for(j = 0; j < list_size(stars); j++){
      if(i != j){
        create_newtonian_gravity(s, G, list_get(stars, i), list_get(stars, j));
      }
    }
  }
}

int random_in_range(int max, int min){
  return ((rand() % (max - min)) + min);
}

int main(void){
  srand(time(0)); // set random seed based on current time
  size_t i;
  Scene *scene = scene_init();
  Body *back = make_rect(max, min);
  List *stars = make_stars();

  scene_add_body(scene, back);
  for(i = 0; i < NUM_BODIES; i++){
    scene_add_body(scene, list_get(stars, i));
  }

  add_gravity(scene, stars);

  sdl_init(min, max);
  while(!sdl_is_done()){
    for (i = 0; i < NUM_BODIES; i++) {
      Body *body = scene_get_body(scene, i);
      Vector cen = body_get_centroid(body);
      if(cen.x < min.x - 2 * BIGGEST) {
          polygon_translate(body->polygon, (Vector){2 * max.x + 2 * BIGGEST, 0});
      }
      else if(cen.x > max.x + 2 * BIGGEST) {
          polygon_translate(body->polygon, (Vector){-2 * max.x - 2 * BIGGEST, 0});
      }
      else if(cen.y < min.y - 2 * BIGGEST) {
        polygon_translate(body->polygon, (Vector){0, max.y * 2 + 2* BIGGEST});
      }
      else if(cen.y > max.y + 2 * BIGGEST) {
          polygon_translate(body->polygon, (Vector){0, max.y * -2 - 2* BIGGEST});
      }
    }
    scene_tick(scene, 1E-8);
    sdl_render_scene(scene,NULL);
  }
  scene_free(scene);
  free(stars->data);
  free(stars);
  return 0;
}
