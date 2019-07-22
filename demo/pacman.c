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
#include <stdio.h>

#define PACMAN_RADIUS 10.0
#define DOT_RADIUS 2.0
#define DOT_RATE 1
#define YELLOW (RGBColor) {1,1,0}
#define INITIAL_NUM_OF_DOTS 10
#define BASE_SPEED 200
#define ACCELERATION 1.01

const Vector min = (Vector) { .x = -200, .y = -100};
const Vector max = (Vector) { .x = 200, .y = 100 };

Body *pacman;

List *generate_pacman(double radius) {
    int i;
    Vector to_add;
    Vector *temp;
    List *points = list_init(102, vec_free);

    for (i = 0; i < 103; i++) {
        temp = (Vector *) malloc(sizeof(Vector));
        assert(temp != NULL);
        if (i == 101) {
            to_add = (Vector) {.x = 0, .y = 0};
            temp->x = to_add.x, temp->y = to_add.y;
            list_add(points, (void *) temp);
        }
        else if (i == 102) {
            to_add = (Vector) {.x = radius * cos(M_PI / 3), .y = radius *sin(M_PI / 3)};
            temp->x = to_add.x, temp->y = to_add.y;
            list_add(points, (void *) temp);
        }
        else {
            to_add = (Vector) {.x = radius * cos(M_PI / 3), .y = radius *sin(M_PI /3 )};
            to_add = vec_rotate(to_add, (i + 1) * (3 * M_PI / 180));
            temp->x = to_add.x, temp->y = to_add.y;
            list_add(points, (void *) temp);
        }
    }
    polygon_rotate(points, 330*M_PI / 180, (Vector) {0,0});
    return points;
}


double random_double(int lower, int upper) {
    return (double)((rand() % (upper - lower + 1)));
}


List *generate_dot(double radius) {
    int i;
    Vector to_add;
    Vector *temp;
    double x = random_double(min.x, max.x) - max.x;
    double y =  1.3 * random_double(min.y, max.y) - 0.5 * max.y;
    List *points = list_init(120, vec_free);
    for (i = 0; i < 120; i++) {
        temp = (Vector *) malloc(sizeof(Vector));
        assert(temp != NULL);
        to_add = (Vector) {0, radius};
        to_add = vec_rotate(to_add, (i + 1)* 3*M_PI / 180);
        temp->x = to_add.x, temp->y = to_add.y;
        list_add(points, (void *) temp);
    }
    polygon_translate(points, (Vector) {x,y});
    return points;
}


List *dots_list(double radius, int num_of_dots) {
    List *dots = list_init(num_of_dots, body_free);
    for (int i = 0; i < num_of_dots; i++) {
        List *point = generate_dot(radius);
        Body *single_point = body_init(point, 0, YELLOW);
        list_add(dots, single_point);
    }
    return dots;
}

Body *generate_rect(Vector top_left, Vector bottom_right){
  List *rect =list_init(4, vec_free);
  Vector *one = malloc(sizeof(Vector));
  Vector *two = malloc(sizeof(Vector));
  Vector *thr = malloc(sizeof(Vector));
  Vector *fou = malloc(sizeof(Vector));

  one->x = top_left.x;
  one->x = top_left.y;
  two->x = top_left.x;
  two->x = bottom_right.y;
  thr->x = bottom_right.x;
  thr->x = bottom_right.y;
  fou->x = bottom_right.x;
  fou->x = top_left.y;

  list_add(rect, (void *)(one));
  list_add(rect, (void *)(two));
  list_add(rect, (void *)(thr));
  list_add(rect, (void *)(fou));

  Body *background = body_init(rect, 0, (RGBColor){0, 0, 0});
  return background;
}


/*#
define LEFT_ARROW 1
#define UP_ARROW 2
#define RIGHT_ARROW 3
#define DOWN_ARROW 4 */
void on_key(char key, KeyEventType type, double held_time, void *oof) {
  if (type == 0) {
         switch (key) {
          case 1:
              if (pacman->velocity.x >= 0) {
                pacman->velocity.x = -BASE_SPEED;
              }
              else {
                pacman->velocity.x -= BASE_SPEED;
              }
              pacman->velocity.y = 0;
              body_set_rotation(pacman, 180 * M_PI/180);
              break;
          case 2:
            if (pacman->velocity.y <= 0) {
              pacman->velocity.y = BASE_SPEED;
            }
            else {
              pacman->velocity.y += BASE_SPEED;
            }
            pacman->velocity.x = 0;
            body_set_rotation(pacman, 90 * M_PI/180);
            break;
          case 3:
            if (pacman->velocity.x <= 0) {
              pacman->velocity.x = BASE_SPEED;
            }
            else {
              pacman->velocity.x += BASE_SPEED;
            }
            pacman->velocity.y = 0;
            body_set_rotation(pacman, 0);
            break;
          case 4:
            if (pacman->velocity.y >= 0) {
              pacman->velocity.y = -BASE_SPEED;
            }
            else {
              pacman->velocity.y -= BASE_SPEED;
            }
            pacman->velocity.x = 0;
            body_set_rotation(pacman, 270 * M_PI/180);
            break;
          }
        }

  if (type == 1) {
    if (held_time != 0) {
       switch (key) {
          case 1:
            if (pacman->velocity.x >= 0) {
              pacman->velocity.x = -BASE_SPEED;
            }
            else {
              pacman->velocity.x *= ACCELERATION * held_time;
            }
            pacman->velocity.y = 0;
            break;
        case 2:
          if (pacman->velocity.y <= 0) {
            pacman->velocity.y = BASE_SPEED;
          }
          else {
            pacman->velocity.y *= ACCELERATION * held_time;;
          }
          pacman->velocity.x = 0;
            break;
        case 3:
          if (pacman->velocity.x <= 0) {
            pacman->velocity.x = BASE_SPEED;
          }
          else {
            pacman->velocity.x *= ACCELERATION * held_time;;
          }
          pacman->velocity.y = 0;
          break;
        case 4:
          if (pacman->velocity.y >= 0) {
            pacman->velocity.y = -BASE_SPEED;
          }
          else {
            pacman->velocity.y *= ACCELERATION * held_time;;
          }
          pacman->velocity.x = 0;
      }
    }
  }
}


void update(Scene *scene, double dt) {
    Vector cen = polygon_centroid(pacman->polygon);
    scene_tick(scene, dt);

    if(cen.x < min.x - 2 * PACMAN_RADIUS) {
        polygon_translate(pacman->polygon, (Vector){2 * max.x + 2 * PACMAN_RADIUS, 0});
    }
    else if(cen.x > max.x + 2 * PACMAN_RADIUS) {
        polygon_translate(pacman->polygon, (Vector){-2 * max.x - 2 * PACMAN_RADIUS, 0});
    }
    else if(cen.y < min.y - 2 * PACMAN_RADIUS) {
      polygon_translate(pacman->polygon, (Vector){0, max.y * 2 + 2* PACMAN_RADIUS});
    }
    else if(cen.y > max.y + 2 * PACMAN_RADIUS) {
        polygon_translate(pacman->polygon, (Vector){0, max.y * -2 - 2* PACMAN_RADIUS});
    }
}

double distance(Vector center, Vector point) {
    double x_diff = center.x - point.x;
    double y_diff = center.y - point.y;

    return sqrt(pow(x_diff, 2) + pow(y_diff, 2));
}

bool inside_pacman(Body *dot) {
    Vector center = polygon_centroid(pacman->polygon);
    Vector point_center = polygon_centroid(dot->polygon);
    double d = distance(center, point_center);
    if (d <= PACMAN_RADIUS) {
      return true;
    }
    return false;
}


int main(void) {
  int i;
  size_t j;
  double dt = 0.0;
  double time_elapsed = DOT_RATE;
  pacman = body_init(generate_pacman(PACMAN_RADIUS), 0, YELLOW);
  List *dots = list_init(INITIAL_NUM_OF_DOTS, body_free);
  dots = dots_list(DOT_RADIUS, INITIAL_NUM_OF_DOTS);
  Scene *scene = scene_init();
  //Body *background = generate_rect(max, min);
  //scene_add_body(scene, background);
  scene_add_body(scene, pacman);
  for (i = 0; i < INITIAL_NUM_OF_DOTS; i++) {
    scene_add_body(scene, list_get(dots, i));
  }

  srand(time(0));
  sdl_init(min, max);
  sdl_on_key(on_key, NULL);

  while (!sdl_is_done()) {
    if (time_elapsed >= DOT_RATE) {
        time_elapsed = 0;
        List *point = generate_dot(DOT_RADIUS);
        Body *single_point = body_init(point, 0, YELLOW);
        scene_add_body(scene, single_point);
    }
    dt = time_since_last_tick();
    time_elapsed += dt;
    update(scene, dt);
    for (j = 1; j < list_size(scene->bodies); j++) {
      if (inside_pacman(scene_get_body(scene, j))) {
        scene_remove_body(scene, j);
        j -= 1;
      }
    }
    sdl_render_scene(scene,NULL);
  }
  scene_free(scene);
  return 0;
}
