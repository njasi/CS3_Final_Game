#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include "sdl_wrapper.h"
#include "forces.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "color.h"

#define PLAYER_WIDTH 10
#define PLAYER_HEIGHT 10
#define PLAYER_SPEED 50
#define PLAYER_COOLDOWN 0.5
#define INVADER_SIZE 10
#define INVADER_SPEED 30
#define INVADER_COOLDOWN 20
#define BULLET_SPEED 120
#define BULLET_SIZE 1

const Vector min = (Vector) { .x = -200, .y = -100};
const Vector max = (Vector) { .x = 200, .y = 100 };

bool game_over = false;

int random_in_range(int max, int min);

int random_in_range(int max, int min){
  return ((rand() % (max - min)) + min);
}

Body *make_background(Vector max, Vector min){
  ship_data *info = malloc(sizeof(ship_data));
  (*info).type = 42069;

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

  return body_init_with_info(rect, 0, (RGBColor){0, 0, 0}, (void *)info, free);
}

Body *make_player(int height, int width){
  ship_data *info = malloc(sizeof(ship_data));
  (*info).type = 0;
  (*info).time_since_last_shot = 0;

  assert(width % 2 == 0);
  List *points = list_init(4, vec_free);

  Vector *temp = malloc(sizeof(Vector));
  temp->x = -width/2;
  temp->y = 0;
  list_add(points, temp);

  temp = malloc(sizeof(Vector));
  temp->x = 0;
  temp->y = height;
  list_add(points, temp);

  temp = malloc(sizeof(Vector));
  temp->x = width/2;
  temp->y = 0;
  list_add(points, temp);

  temp = malloc(sizeof(Vector));
  temp->x = 0;
  temp->y = height/3;
  list_add(points, temp);
  polygon_translate(points, (Vector){0, min.y + PLAYER_HEIGHT});
  return body_init_with_info(points, 0, (RGBColor){0.224,1 ,.078}, (void *)info, free);
}

Body *make_invader(int radius){
  int arc_points = 120;
  double ang_i = (120 / arc_points) * M_PI / 180;
  ship_data *info = malloc(sizeof(ship_data));
  (*info).type = 1;
  double start = random_in_range(INVADER_COOLDOWN, 0);
  (*info).time_since_last_shot = start;
  int i;
  Vector to_add;
  Vector *temp;
  List *points = list_init(arc_points + 1, vec_free);

  for (i = 0; i < arc_points; i++) {
      to_add = (Vector){INVADER_SIZE, 0};
      to_add = vec_rotate(to_add, ang_i * (arc_points - i));
      temp = (Vector *) malloc(sizeof(Vector));
      assert(temp != NULL);
      temp->x = to_add.x;
      temp->y = to_add.y;
      list_add(points, (void *)temp);
  }
  temp = (Vector *) malloc(sizeof(Vector));
  temp->x = 0;
  temp->y = 0;
  list_add(points, temp);
  polygon_rotate(points, M_PI/6, polygon_centroid(points));
  Body *invader = body_init_with_info(points, 0, (RGBColor){0.5411, 0.0274 ,0.0274},
                                      (void *)info, free);
  invader->velocity.x = INVADER_SPEED;
  return invader;
}

List *make_invaders(){
  int col_num = (max.x - min.x)/(INVADER_SIZE*2 + 5) - 1;
  int row_num = 3;
  int i, j;
  List *invaders = list_init(row_num * col_num, body_free);
  Body * temp;
  for(i = 0; i<row_num; i++){
    for(j=0; j<col_num; j++){
      temp = make_invader(INVADER_SIZE);
      polygon_translate(temp->polygon, (Vector){j * (INVADER_SIZE * 2 +5) + min.x,
                                                i * INVADER_SIZE * 2 + max.y - INVADER_SIZE * 6});
      list_add(invaders, temp);
    }
  }
  return invaders;
}

Body *make_bullet(RGBColor color, Vector location, void *info){
  List *circle = polygon_ball(BULLET_SIZE, 24);
  polygon_translate(circle, location);
  return body_init_with_info(circle, 0, color, info, free);
}

void shoot(Scene *game, Body *ship){
  // this gets us the cool effect of shooting down bullets too
  int type = (*((ship_data *)ship->info)).type;
  ship_data *temp = malloc(sizeof(ship_data));
  (*temp).type = type + 2;
  Body *bullet = make_bullet(ship->fill_color, polygon_centroid(ship->polygon), temp);
  switch (type) {
    case 0: // player shooting
      bullet->velocity.y = BULLET_SPEED;
      break;
    case 1:
      bullet->velocity.y = -1 * BULLET_SPEED;  // enemy shooting
      break;
    default:
      bullet->velocity.x = BULLET_SPEED;
  }

  size_t i = 0;
  int info;
  for(;i < scene_bodies(game); i++){
    info = (*((ship_data *)(scene_get_body(game, i)->info))).type;
    if((info != type && (info != type + 2)) && info != 42069){ // 42069 represents the background
      // printf("%s\n", "ADD COLLISION");
      create_destructive_collision(game, bullet, scene_get_body(game, i)); //TODO
    }
  }
  scene_add_body(game, bullet);
}

void on_key(char key, KeyEventType type, double held_time, void *data){
  Scene *game = (Scene *)data;
  Body *player = scene_get_body(game, 1);
  double cooldown = ((ship_data *)(player->info))->time_since_last_shot;
  if(type == 0){//key down
    switch (key) {
      case 1:
        player->velocity.x = -1 * PLAYER_SPEED;  //left
        break;
      case 2: break;
      case 3:
        player->velocity.x = PLAYER_SPEED;  //right
        break;
      case 4: break;
      case 5:
        if(cooldown > PLAYER_COOLDOWN){
          shoot(game, player); //up
          ((ship_data *)(player->info))->time_since_last_shot = 0;
        }
    }
  }
  else if(type == 1){
    switch (key) {
      case 1: player->velocity.x = 0;  //left
      case 3: player->velocity.x = 0;  //right
      // nothing because we use damping to decel player instead of key remove to stop moving
    }
  }
}

void update_things(Scene *game, double dt){
  size_t i = 0;
  ship_data *info;
  double edge = (INVADER_SIZE * cos(M_PI/12));
  Body *temp;
  game_over = true;
  for(; i<scene_bodies(game); i++){
    temp = scene_get_body(game, i);
    info = (ship_data *)(temp->info);
    if(info->type == 1){ //indicates an invader
      if(polygon_centroid(temp->polygon).x > max.x - edge){
        polygon_translate(temp->polygon, (Vector){-1 *(max.x - min.x - (2 * edge)),
                                                  INVADER_SIZE * -2});
      }
      info->time_since_last_shot += dt;
      if(info->time_since_last_shot > INVADER_COOLDOWN){
        info->time_since_last_shot = (double)random_in_range(INVADER_COOLDOWN/4, 0);
        shoot(game, temp);
      }
    }else if(info->type == 0){
      game_over = false;
      info->time_since_last_shot += dt;
    }else if(info->type == 2){
      if(body_get_centroid(temp).y > max.y){
        remove_flag(scene_get_body(game, i));
      }
    }else if(info->type == 3){
      if(body_get_centroid(temp).y < min.y){
        remove_flag(scene_get_body(game, i));
      }
    }
  }
}



int main(void) {
  srand(time(0)); //set random seed
  double dt = 0;
  Scene *scene = scene_init();
  Body* player = make_player(PLAYER_HEIGHT, PLAYER_WIDTH);
  scene_add_body(scene, make_background(max, min));
  scene_add_body(scene, player);
  int i = 0;
  List *invaders = make_invaders();
  for(;i<list_size(invaders); i++){
    scene_add_body(scene, (Body *)list_get(invaders, i));
  }

  sdl_init(min, max);
  sdl_on_key(on_key, (void *)scene);

  while (!sdl_is_done()) {
    dt = time_since_last_tick();
    scene_tick(scene, dt);
    update_things(scene, dt);
    sdl_render_scene(scene, NULL);
    if (game_over) {
      break;
    }
  }
  scene_free(scene);
  free(invaders->data);
  free(invaders);
  return 0;
}
