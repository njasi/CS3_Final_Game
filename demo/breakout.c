#include <stdlib.h>
#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <limits.h>
#include <stdbool.h>
#include <stdio.h>
#include "sdl_wrapper.h"
#include "forces.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "color.h"

#define PLAYER_WIDTH 40
#define PLAYER_HEIGHT 2
#define PLAYER_SPEED 200
#define PLAYER_COOLDOWN 0.5
#define BLOCK_WIDTH 20
#define BLOCK_HEIGHT 3
#define BALL_SPEED PLAYER_SPEED / 1.5
#define BALL_SIZE 3
#define BALL_MASS 5
#define ROW_NUM 5

const Vector min = (Vector) { .x = -200, .y = -100 };
const Vector max = (Vector) { .x = 200, .y = 100 };

int random_in_range(int max, int min);

int random_in_range(int max, int min){
  return ((rand() % (max - min)) + min);
}

Body *make_background(Vector max, Vector min){
  block_data *info = malloc(sizeof(block_data));
  (*info).type = 42069;
  List *rect = polygon_rect(max, min);
  return body_init_with_info(rect, 0, (RGBColor){0, 0, 0}, (void *)info, free);
}

Body *make_player(){
  block_data *info = malloc(sizeof(block_data));
  (*info).type = 0;

  List *points = polygon_rect((Vector){0,0},(Vector){PLAYER_WIDTH, PLAYER_HEIGHT});
  polygon_translate(points, (Vector){-PLAYER_WIDTH/2, min.y + PLAYER_HEIGHT});

  return body_init_with_info(points, INT_MAX, (RGBColor){0.224,1 ,.078}, (void *)info, free);
}

Body *make_block(int width, int height, int hits){
  block_data *info = malloc(sizeof(block_data));
  (*info).type = 2;
  (*info).hits = hits;
  (*info).max_health = hits;
  List *points = polygon_rect((Vector){0,0}, (Vector){BLOCK_WIDTH, BLOCK_HEIGHT});
  Body *block = body_init_with_info(points, INT_MAX, (RGBColor){0,0,0}, (void *)info, free);
  return block;
}

List *make_blocks(){
  int col_num = (max.x - min.x)/(BLOCK_WIDTH*2);
  int i, j;
  List *blocks = list_init(ROW_NUM * col_num, body_free);
  Body * temp;
  for(i = 0; i < ROW_NUM; i++){
    for(j = 0; j < col_num; j++){
      temp = make_block(BLOCK_WIDTH, BLOCK_HEIGHT, ROW_NUM - i);
      temp->fill_color = color_rainbow(col_num, j);
      polygon_translate(temp->polygon, (Vector){2 * j * BLOCK_WIDTH + min.x + BLOCK_WIDTH / 2,
                                                 max.y - 8 * i * BLOCK_HEIGHT - 6 * BLOCK_HEIGHT});
      list_add(blocks, temp);
    }
  }
  return blocks;
}

Body *make_ball(double radius){
  block_data *info = malloc(sizeof(block_data));
  (*info).type = 1;
  List *points = polygon_ball(radius,120);
  return body_init_with_info(points, BALL_MASS, color_rand(), (void *)info, free);
}

void on_key(char key, KeyEventType type, double held_time, void *data){
  Body *player = (Body *)data;
  if(type == 0){ // key down
    switch (key) {
      case 1: // left
        player->velocity.x = -1 * PLAYER_SPEED;
        break;
      case 3: // right
        player->velocity.x = PLAYER_SPEED;
        break;
    }
  }
  else if(type == 1){
    switch (key) {
      case 1: // left
        player->velocity.x = 0;
        break;
      case 3: // right
        player->velocity.x = 0;
        break;
    }
  }
}

// the collision handler called when the ball impacts to damage the blocks
void damage(Body *body1, Body *body2, Vector axis, void *aux) {
  block_data *info = (block_data *) body2->info;
  info->hits--;
  if (info->hits == 0) {
    remove_flag(body2);
  }
  body1->colliding = false, body2->colliding = false;
}

void reset_game(Scene *game){
  int i = 6; // back, player, ball, and walls need no reset
  for(;i<scene_bodies(game);i++){
    remove_flag(scene_get_body(game, i));
  }
  // player and ball stuff
  body_set_centroid(scene_get_body(game, 1), (Vector){0,min.y + PLAYER_HEIGHT/2}); //player
  Body *ball = scene_get_body(game, 2); //ball
  body_set_centroid(ball, (Vector){-PLAYER_WIDTH/2, min.y + 4 * PLAYER_HEIGHT});
  ball->velocity = (Vector){10, BALL_SPEED};

  // add back blocks
  List *blocks = make_blocks();
  for(;i<list_size(blocks); i++){
    scene_add_body(game, (Body *)list_get(blocks, i));
    create_collision(game, ball, (Body *) list_get(blocks, i), damage, NULL, NULL);
    create_physics_collision(game, 1, ball, (Body *) list_get(blocks, i));
  }
}

bool check_over(Body *ball){
  return body_get_centroid(ball).y < min.y - BALL_SIZE || body_get_centroid(ball).y > max.y + BALL_SIZE;
}

int main(void) {
  srand(time(0)); //set random seed
  bool end = false;
  RESTART: do {
    double dt = 0;
    Scene *scene = scene_init();
    Body* player = make_player();
    Body* ball = make_ball(BALL_SIZE);
    body_set_centroid(ball, (Vector){-PLAYER_WIDTH/2, min.y + 4 * PLAYER_HEIGHT});
    ball->velocity = (Vector){10, BALL_SPEED};
    Body* right_wall = body_init(polygon_rect((Vector){max.x,max.y},(Vector){max.x + 10,min.y}),  INT_MAX,(RGBColor){0,0,0});
    Body* left_wall = body_init(polygon_rect((Vector){min.x,min.y},(Vector){min.x - 10,max.y}),INT_MAX,(RGBColor){0,0,0});
    Body *top_wall = body_init(polygon_rect((Vector){min.x,max.y},(Vector){max.x, max.y + 10}), INT_MAX, (RGBColor) {0,0,0});
    scene_add_body(scene, make_background(max, min));
    scene_add_body(scene, player);
    scene_add_body(scene, ball);
    scene_add_body(scene, right_wall);
    scene_add_body(scene, left_wall);
    scene_add_body(scene, top_wall);

    create_physics_collision(scene, 1, ball, player);
    create_physics_collision(scene, 1, ball, left_wall);
    create_physics_collision(scene, 1, ball, right_wall);
    create_physics_collision(scene, 1, ball, top_wall);

    int i = 0;
    List *blocks = make_blocks();
    for(;i < list_size(blocks); i++){
      scene_add_body(scene, (Body *)list_get(blocks, i));
      create_collision(scene, ball, (Body *) list_get(blocks, i), damage, NULL, NULL);
      create_physics_collision(scene, 1, ball, (Body *) list_get(blocks, i));
    }

    sdl_init(min, max);
    sdl_on_key(on_key, (void *)player);

    while (!sdl_is_done()) {
      dt = time_since_last_tick();
      scene_tick(scene, dt);
      sdl_render_scene(scene,NULL);
      if(check_over(ball)){
        sdl_destroy();
        scene_free(scene);
        free(blocks->data);
        free(blocks);
        goto RESTART;
      }
    }
    end = true;
    scene_free(scene);
    free(blocks->data);
    free(blocks);
  } while(!end);
  return 0;
}
