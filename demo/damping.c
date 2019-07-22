#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <limits.h>
#include <stdio.h>
#include "sdl_wrapper.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "color.h"
#include "forces.h"

#define MASS 100
#define BALL_RADIUS 5
#define DRAG_FACTOR 2
#define SPRING_FACTOR 9
const Vector min = (Vector) { .x = -200, .y = -100};
const Vector max = (Vector) { .x = 200, .y = 100 };

size_t num_balls;

List *make_ball(double radius, size_t num_point) {
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

List *make_balls(){
  List *balls = list_init(num_balls, body_free);
  size_t i = 0;
  for(;i < num_balls; i++){
    List *ball = make_ball(BALL_RADIUS, 120);
    polygon_translate(ball, (Vector){i * 2 * BALL_RADIUS +
                                     min.x + BALL_RADIUS, 0});
    Body *temp = body_init(ball, MASS, color_rainbow(num_balls, i));
    list_add(balls, (void *)temp);
  }
  return balls;
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

void format_balls(Scene *s, List *balls, List *anchors, bool reverse, int start_y){
  size_t i = 0;
  double drag = 0.0;
  for(;i < list_size(balls); i++){
    polygon_translate(((Body *)list_get(balls, i))->polygon, (Vector){0, start_y});
      if(reverse){
        drag = DRAG_FACTOR * ((double)list_size(balls)-i)/((double)list_size(balls));
        ((Body *)list_get(balls, i))->fill_color = color_rainbow(list_size(balls), list_size(balls)-i);
      }else{
        drag = DRAG_FACTOR * ((double)i)/((double)list_size(balls));

      }
    create_spring(s, SPRING_FACTOR, (Body *) list_get(balls, i), (Body *) list_get(anchors, i));
    create_drag(s, drag, (Body *) list_get(balls, i));
  }
}

List *make_anchors(){
  List *anchors = make_balls();
  size_t i = 0;
  for(;i< list_size(anchors);i++){
    ((Body *)list_get(anchors,i))->mass = INT_MAX;
    ((Body *)list_get(anchors,i))->fill_color = (RGBColor){0, 0, 0};
  }
  return anchors;
}

int main(void){
  srand(time(0));
  size_t i;
  num_balls = (size_t)((max.x + fabs(min.x)) / (2 * BALL_RADIUS));

  Scene *scene = scene_init();
  List *balls = make_balls();
  List *balls_r = make_balls();
  List *anchors = make_anchors();
  Body *back = make_rect(max, min);
//oof
  scene_add_body(scene, back);
  for(i = 0; i < list_size(anchors); i++) {
    scene_add_body(scene, list_get(anchors, i));
    scene_add_body(scene, list_get(balls_r, i));
    scene_add_body(scene, list_get(balls, i));
  }

  format_balls(scene, balls, anchors, false, max.y - BALL_RADIUS);
  format_balls(scene, balls_r, anchors, true, min.y + BALL_RADIUS);

  sdl_init(min, max);
  while(!sdl_is_done()) {
    for (i = 0; i < num_balls; i++) {
      Body *body = scene_get_body(scene, i);
      Vector cen = body_get_centroid(body);
      if(cen.x < min.x - 2 * BALL_RADIUS) {
          polygon_translate(body->polygon, (Vector){2 * max.x + 2 * BALL_RADIUS, 0});
      }
      else if(cen.x > max.x + 2 * BALL_RADIUS) {
          polygon_translate(body->polygon, (Vector){-2 * max.x - 2 * BALL_RADIUS, 0});
      }
      else if(cen.y < min.y - 2 * BALL_RADIUS) {
        polygon_translate(body->polygon, (Vector){0, max.y * 2 + 2* BALL_RADIUS});
      }
      else if(cen.y > max.y + 2 * BALL_RADIUS) {
          polygon_translate(body->polygon, (Vector){0, max.y * -2 - 2* BALL_RADIUS});
      }
    }
    scene_tick(scene, 1E-1);
    sdl_render_scene(scene,NULL);
  }

  scene_free(scene);
  free(balls->data);
  free(balls);
  free(balls_r->data);
  free(balls_r);
  free(anchors->data);
  free(anchors);
  return 0;
}
