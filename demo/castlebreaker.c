/**
 * CASTLE BREAKERS CDXX LXIX ;)
 * By Nick J, Archie S, Marcus G, and Gokul S
 *
 * Sections (ctrl-f to them)
 *  - Structs
 *  - Input Stuff
 *  - Object Stuff
 *    - Collisions
 *  - Game Init
 *    - Cannon Positions
 *    - Bounding Wall
 *    - Background Stuff
 *    - Cannon Stuff
 *    - Power Bar Stuff
 *    - Wind Stuff
 *  - Networking
 *  - Map Stuff
 *  - Game Loop
 *    - Change Wind
 */

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <time.h>
#include <stdbool.h>
#include <stdio.h>
#include <limits.h>
#include <errno.h>
#include <unistd.h>
#include "sdl_wrapper.h"
#include "forces.h"
#include "polygon.h"
#include "body.h"
#include "list.h"
#include "scene.h"
#include "color.h"
#include "network_engine.h"
#include "tileset.h"

// Parameters
#define BLOCK_SIZE        8
#define BALL_SIZE         5
#define BALL_MASS         1
#define G                98
#define SCROLL_SPEED     20
#define SPEED_LIMIT     800
#define POWERBALL_SPEED  40
#define GAMMA           100

const Vector min = (Vector) { .x = -400, .y = -200 };
const Vector max = (Vector) { .x  = 400, .y =  200 };

// Prototypes
void fire(Scene *scene, List *walls, void *data);
double random_in_range(int max, int min);
void update_projectiles();

///////////////////////////
//        Structs        //
///////////////////////////

typedef struct {
  int **data;
  int rows;
  int cols;
  char *tileset_path;
  List *tileset_data;
  List *tileset_meta;
} MapData;

typedef struct {
  Scene *scene;
  List *walls;
  void *data;
} fire_args;

typedef struct {
  char *filename;
  Vector cannon1;
  Vector cannon2;
} cannon_positions;

typedef struct {
  char* texture;
  int ball_num1;
  int ball_num2;
} display_ball;

typedef struct {
  int conn;
  int body_count;
  bool turn;
  bool mouse_state;
  Body *power_bar;
  Body *power_ball;
  Body *power_bar2;
  Body *power_ball2;
  Body *proj_display1;
  Body *proj_display2;
  double speed;
  double speed2;
  Vector ca;
  double gamma;
  char *gamma_string;
  cannon_positions positions[9];
  display_ball projectiles[5];
  display_ball projectile;
  int projectile_idx;
  int ball_num_turn1;
  int ball_num_turn2;
  char *ball_num_str1;
  char *ball_num_str2;
  Mix_Chunk *firing;
  Mix_Chunk *fuse;
  Mix_Music *music;
  List *damage_texture;
  int game_over;
  bool locked;
  bool networked;
  char *status;
} game_state;

typedef struct {
  int type;
  int current_health;
  int max_health;
  CollisionType collision;
  bool damaged;
} BodyInfo;

///////////////////////////
//      End Structs      //
///////////////////////////

game_state state;

double random_in_range(int max, int min) {
  srand(time(0));
  double random;
  random = (double)((rand() % (max - min) + min));
  return random;
}

char* get_display_ball_char(display_ball ball) {
  return(ball.texture);
}

int get_display_ball_int1(display_ball ball) {
  return(ball.ball_num1);
}

void BodyInfo_init(Body *body, int type) {
  BodyInfo *info = malloc(sizeof(BodyInfo));
  info->type = type;
  info->damaged = false;
  body->info = (void *) info, body->freer = free;
}

BodyInfo *BodyInfo_init_tile(MapData *map, int c) {
  BodyInfo *info = malloc(sizeof(BodyInfo));
  info->type = tileset_get_id(map->tileset_data, c);
  info->max_health = tileset_get_health(map->tileset_data, c);
  info->current_health = info->max_health;
  info->collision = tileset_get_collision(map->tileset_data, c);
  return info;
}

void update_projectiles() {
  state.projectile = state.projectiles[state.projectile_idx];
  if (state.turn) {
    state.ball_num_turn1 = state.projectile.ball_num1;
    sprintf(state.ball_num_str1, "%d", state.ball_num_turn1);
    char *texture = get_display_ball_char(state.projectile);
    body_set_texture(state.proj_display1, sdl_load_image(texture));
  }
  else {
    state.ball_num_turn2 = state.projectile.ball_num2;
    sprintf(state.ball_num_str2, "%d", state.ball_num_turn2);
    char *texture = get_display_ball_char(state.projectile);
    body_set_texture(state.proj_display2, sdl_load_image(texture));
  }
}

///////////////////////////
//      Input Stuff      //
///////////////////////////

void on_key(char key, KeyEventType type, double held_time, void *data) {
  // Get information for which cannon to fire
  List *list = (List *) data;
  fire_args *fa1 = (fire_args *) list_get(list, 0);
  fire_args *fa2 = (fire_args *) list_get(list, 1);

  // Key pressed
  if (type == 0) {
    switch (key) {
      case LEFT_ARROW:
        sdl_change_scroll((Vector) {SCROLL_SPEED, 0}, SPEED_LIMIT);
        break;
      case UP_ARROW:
        sdl_change_scroll((Vector) {0, SCROLL_SPEED}, SPEED_LIMIT);
        break;
      case RIGHT_ARROW:
        sdl_change_scroll((Vector) {-1 * SCROLL_SPEED, 0}, SPEED_LIMIT);
        break;
      case DOWN_ARROW:
        sdl_change_scroll((Vector) {0, -1 * SCROLL_SPEED}, SPEED_LIMIT);
        break;
      case SPACEBAR:
        if (state.locked) {
          if (state.turn && ((state.networked && strcmp(state.status, "server") == 0) || !state.networked)) {
            // Stop power ball and extract speed
            body_set_velocity(state.power_ball, VEC_ZERO);
            state.speed = body_get_centroid(state.power_ball).x + 540;
            
            // Select correct projectile
            if (state.projectile.ball_num1 > 0) {
              fire(fa1->scene, fa1->walls, fa1->data);
            }
            if (state.projectiles[state.projectile_idx].ball_num1 > 0) {
              state.projectiles[state.projectile_idx].ball_num1 -= 1;
              state.turn = !state.turn;
              nu_send_str(state.conn, "SWITCHTURN");
            }
            state.projectile = state.projectiles[state.projectile_idx];
            state.ball_num_turn1 = state.projectile.ball_num1;

            // Reset power ball and load correct texture
            int translate = min.x + 110 - body_get_centroid(state.power_ball).x;
            body_set_centroid(state.power_ball, (Vector){translate, 0});
            state.projectile_idx = 0;
            state.projectile = state.projectiles[state.projectile_idx];
            state.ball_num_turn1 = state.projectile.ball_num1;
            sprintf(state.ball_num_str1, "%d", state.ball_num_turn1);
            char* texture = get_display_ball_char(state.projectile);
            body_set_texture(state.proj_display1, sdl_load_image(texture));
            state.locked = false;
          }
          else if (!state.turn && ((state.networked && strcmp(state.status, "client") == 0) || !state.networked)) {
            body_set_velocity(state.power_ball2, VEC_ZERO);
            state.speed2 = 250-(body_get_centroid(state.power_ball2).x -40) + 250;
        
            if (state.projectile.ball_num2 > 0) {
              fire(fa2->scene, fa2->walls, fa2->data);
            }
            if (state.projectiles[state.projectile_idx].ball_num2 > 0) {
              state.projectiles[state.projectile_idx].ball_num2 -= 1;
              state.turn = !state.turn;
              nu_send_str(state.conn, "SWITCHTURN");
            }
            state.projectile = state.projectiles[state.projectile_idx];

            int translate = max.x - 110 - body_get_centroid(state.power_ball2).x;
            body_set_centroid(state.power_ball2, (Vector){translate, 0});
            state.projectile_idx = 0;
            state.projectile = state.projectiles[state.projectile_idx];
            state.ball_num_turn2 = state.projectile.ball_num2;
            sprintf(state.ball_num_str2, "%d", state.ball_num_turn2);
            char* texture = get_display_ball_char(state.projectile);
            body_set_texture(state.proj_display2, sdl_load_image(texture));
            state.locked = false;
          }
        }
        break;

      case EQUALS:
        sdl_set_zoom_rate(0.01);
        break;
      case MINUS:
        sdl_set_zoom_rate(-0.01);
        break;
      case TAB:
        if (state.locked) {
          if ((state.turn && ((state.networked && strcmp(state.status, "server") == 0) || !state.networked)) ||
              (!state.turn && ((state.networked && strcmp(state.status, "client") == 0) || !state.networked)))
          {
            // Cycle through projectiles
            state.projectile_idx = ++state.projectile_idx % 5;
            nu_send_str(state.conn, "CHANGE");
            // Update projectiles
            update_projectiles();
          }
        }
        break;
    }
  }

  // Key released
  else if (type == 1) {
    switch (key) {
      case UP_ARROW:
      case DOWN_ARROW:
      case LEFT_ARROW:
      case RIGHT_ARROW:
        sdl_set_scroll(VEC_ZERO);
        break;
      case EQUALS:
      case MINUS:
        sdl_set_zoom_rate(0);
    }
  }
}

void on_mouse(int x, int y, MouseEventType type, void *data) {
  // Get cannon data for angling
  List *key_info = (List *) data;
  Body *barrel1 = (Body *) list_get(key_info, 0);
  Body *barrel2 = (Body *) list_get(key_info, 1);
  int* stopper= (int *) list_get(key_info, 2);
  Vector cannon_center;

  if (state.locked) {
    switch (type) {
      case MOUSEMOTION:
        if (state.mouse_state) {
          // Calculate the angle from centroid of cannon to cursor
          if (state.turn && ((state.networked && strcmp(state.status, "server") == 0) || !state.networked)) {
            cannon_center = body_get_centroid(barrel1);
            double angle = atan2(-1 * y - (min.y + cannon_center.y), x - (max.x + cannon_center.x));
            body_set_rotation(barrel1, angle);
            char command[50];
            sprintf(command, "ROTATE 0 %f", angle);
            nu_send_str(state.conn, command);
          } 
          else if (!state.turn && ((state.networked && strcmp(state.status, "client") == 0) || ! state.networked)) {
            cannon_center = body_get_centroid(barrel2);
            double angle = M_PI + atan2(-1 * y - (min.y + cannon_center.y), x - (max.x + cannon_center.x));
            body_set_rotation(barrel2, angle);
            char command[50];
            sprintf(command, "ROTATE 1 %f", angle);
            nu_send_str(state.conn, command);
          }
        }
        break;
      case MOUSEBUTTONDOWN: // mouse click
        // Only plays sound if correct turn
        if ((state.turn && ((state.networked && strcmp(state.status, "server") == 0) || !state.networked)) ||
            (!state.turn && ((state.networked && strcmp(state.status, "client") == 0) || !state.networked)))
        {
          if (*stopper == -1) {
            *stopper = play_audio_chunk(state.fuse, -1);
          }
        }
        // Only calculate angles when the mouse is held
        state.mouse_state = true;
        break;  
      case MOUSEBUTTONUP:
        stop_channel(*stopper, 0);
        *stopper = -1;
        // Set to false to prevent angle-changing when mouse is up
        state.mouse_state = false;

        // Start power bar when mouse is released
        if (state.turn && ((state.networked && strcmp(state.status, "server") == 0) || !state.networked)) {
          if (body_get_velocity(state.power_ball).x < 0) {
            body_set_velocity(state.power_ball, (Vector){-POWERBALL_SPEED,0});
          }
          else {
            body_set_velocity(state.power_ball, (Vector){POWERBALL_SPEED,0});
          }
        }
        else if (!state.turn && ((state.networked && strcmp(state.status, "client") == 0) || !state.networked)) {
          if (body_get_velocity(state.power_ball2).x == 0) {
            body_set_velocity(state.power_ball2, (Vector){-POWERBALL_SPEED,0});
          }
          else if (body_get_velocity(state.power_ball2).x < 0) {
            body_set_velocity(state.power_ball2, (Vector){-POWERBALL_SPEED,0});
          }
          else {
            body_set_velocity(state.power_ball2, (Vector){POWERBALL_SPEED,0});
          }
        }
        break;
    }

  }

}

///////////////////////////
//    End Input Stuff    //
///////////////////////////

///////////////////////////
//      Object Stuff     //
///////////////////////////

Body *make_ball(double radius) {
  List *points = polygon_ball(radius, 120);
  return body_init(points, BALL_MASS, (RGBColor){1,0,0});
}

Body *add_cannon(Scene *game, Vector pos, bool left) {
  // Init polygons for cannon placement
  Body *barrel  = body_init(polygon_rect(VEC_ZERO, (Vector){BLOCK_SIZE * 4,BLOCK_SIZE * 4}), 10, color_rand());
  Body *bf_body = body_init(polygon_rect(VEC_ZERO, (Vector){BLOCK_SIZE * 4,BLOCK_SIZE * 4}), 10, color_rand());
  Body *bb_body = body_init(polygon_rect(VEC_ZERO, (Vector){BLOCK_SIZE * 4,BLOCK_SIZE * 4}), 10, color_rand());

  BodyInfo_init(barrel, 69);
  BodyInfo_init(bf_body, 69);
  BodyInfo_init(bb_body, 69);

  body_set_id(barrel, state.body_count);
  state.body_count++;

  body_set_size(barrel,  (Vector){BLOCK_SIZE * 4, BLOCK_SIZE * 4});
  body_set_size(bf_body, (Vector){BLOCK_SIZE * 4, BLOCK_SIZE * 4});
  body_set_size(bb_body, (Vector){BLOCK_SIZE * 4, BLOCK_SIZE * 4});

  // Draw correct cannon textures based on orientation
  if (left) {
    body_set_texture(barrel,  sdl_load_image("resources/graphics/cannon/barrel_left.png"));
    body_set_texture(bf_body, sdl_load_image("resources/graphics/cannon/base_front_left.png"));
    body_set_texture(bb_body, sdl_load_image("resources/graphics/cannon/base_back_left.png"));
  } else {
    body_set_texture(barrel,  sdl_load_image("resources/graphics/cannon/barrel_right.png"));
    body_set_texture(bf_body, sdl_load_image("resources/graphics/cannon/base_front_right.png"));
    body_set_texture(bb_body, sdl_load_image("resources/graphics/cannon/base_back_right.png"));
  }

  // Position bodies
  body_set_centroid(barrel,  vec_subtract(pos, (Vector){0, BLOCK_SIZE * 4}));
  body_set_centroid(bf_body, vec_subtract(pos, (Vector){0, BLOCK_SIZE * 4}));
  body_set_centroid(bb_body, vec_subtract(pos, (Vector){0, BLOCK_SIZE * 4}));

  // Add to scene
  scene_add_body(game, bb_body);
  scene_add_body(game, barrel);
  scene_add_body(game, bf_body);

  return barrel;
}

Body *make_block(int width, int height, int hits){
  List *points = polygon_rect(VEC_ZERO, (Vector){BLOCK_SIZE, BLOCK_SIZE});
  Body *block = body_init(points, INT_MAX, (RGBColor){0,0,0});
  return block;
}

Body *make_background(Vector max, Vector min){
  BodyInfo *info = malloc(sizeof(BodyInfo));
  info->type = 42069;
  List *rect = polygon_rect(max, min);
  return body_init_with_info(rect, 0, (RGBColor){0, 0, 0}, (void *) info, free);
}

void damage_overlay_texture(Body *block, double health, void *aux) {
  List *overlay = (List *) aux;
  health *= 100;

  if (health <= 100 && health > 67) {
    block->damage_overlay = tileset_get_texture(overlay, 0);

  }
  else if (health <= 67 && health > 33) {
    block->damage_overlay = tileset_get_texture(overlay, 1);

  }
  else if (health <= 33 && health > 0) {
    block->damage_overlay = tileset_get_texture(overlay, 2);
  }
}

void damage(Body *body1, Body *body2, Vector axis, void *aux) {
  List *overlay = (List *) aux;
  play_audio_chunk(state.firing, 0);

  BodyInfo *info1 = body_get_info(body1);
  if (info1->damaged) {
    body1->colliding = false, body2->colliding = false;
    return;
  }

  BodyInfo *info2 = body_get_info(body2);
  info2->current_health--;
  if (info2->type == 10) {
    state.game_over = 1;
  }

  if (info2->type == 11) {
    state.game_over = 2;
  }

  if (state.projectile_idx == 3) {
    info2->current_health--;
  }

  double health_percent = (double) info2->current_health / info2->max_health;
  damage_overlay_texture(body2, health_percent, overlay);

  if (health_percent == 0) {
    remove_flag(body2);
  }
  body1->colliding = false, body2->colliding = false;
  info1->damaged = true;
  state.locked = true;
}

void ball_remove(Body *body1, Body *body2, Vector axis, void *aux) {
  remove_flag(body1);
  state.locked = true;
}

void cannon_ball(Scene *scene, List *walls, void *data) {
  size_t i;
  Body *body = make_ball(BALL_SIZE);
  Body *center_body = (Body *) data;
  Vector velocity;
  body_set_centroid(body, body_get_centroid(center_body));
  double angle = center_body->angle;

  // Impart correct velocity based on power bar and angle
  if (state.turn) {
    if (state.projectile_idx != 1) {
      velocity = (Vector) {(state.speed + state.gamma) * cos(angle), state.speed * sin(angle)};
      }
    else {
      velocity = (Vector) {(state.speed) * cos(angle), state.speed * sin(angle)};
      }
    }
  else {
    if (state.projectile_idx != 1) {
      velocity = (Vector) {-1 * (state.speed2 + state.gamma) * cos(angle), -1 * state.speed2 * sin(angle)};
      }
    else {
      velocity = (Vector) {-1 * state.speed2 * cos(angle), -1 * state.speed2 * sin(angle)};
    }
  }

  body_set_velocity(body, velocity);
  body_set_id(body, state.body_count);
  body->angular_speed = 5;
  char *texture = get_display_ball_char(state.projectile);
  body_set_texture(body, sdl_load_image(texture));
  BodyInfo_init(body, 69);
  body_set_size(body, (Vector) {2 * BALL_SIZE, 2 * BALL_SIZE});

  ///////////////////////////
  //      Collisions       //
  ///////////////////////////

  i = scene_bodies(scene);
  while (i --> 5) {
    Body *object = scene_get_body(scene, i);
    BodyInfo *info = (BodyInfo *) body_get_info(object);
    // Skip if non-interacting object
    if (info->type == 69) {
      continue;
    }
    switch (info->collision) {
    case GHOST:
      break;
    case INVINCIBLE:
      create_collision(scene, body, object, ball_remove, NULL, NULL);
      break;
    case DESTRUCTIVE:
      create_destructive_collision(scene, body, object);
      break;
    case DAMAGE:
      create_collision(scene, body, object, damage, (void *) state.damage_texture, NULL);
      create_physics_collision(scene, 0.25, body, object);
      break;
    }
  }

  i = list_size(walls);
  while (i --> 0) {
    create_collision(scene, body, (Body *) list_get(walls, i), ball_remove, NULL, NULL);
    state.locked = true;
  }

  ///////////////////////////
  //     End Collisions    //
  ///////////////////////////

  scene_add_body(scene, body);
  if (state.projectile_idx != 2) {
    create_flat_earth(scene, G, body);
  } else {
    body->angular_speed = 0;
    if(state.turn) {
      body_set_rotation(body, angle + M_PI / 2);
    } else {
      body_set_rotation(body, angle);
    }
  }
  state.gamma = random_in_range(GAMMA, -GAMMA);
}

///////////////////////////
//    End Object Stuff   //
///////////////////////////

///////////////////////////
//       Game Init       //
///////////////////////////

void fire(Scene *scene, List *walls, void *data) {
  cannon_ball(scene, walls, data);
  Vector v = body_get_velocity(scene_get_body_id(scene, state.body_count));
  char command[50];
  sprintf(command, "ADDOBJECT %d %d VECTOR[%f,%f]", state.body_count, state.turn ? 0 : 1, v.x, v.y);
  nu_send_str(state.conn, command);
  play_audio_chunk(state.firing, 0);
  state.body_count++;
}

MapData *load_map(char *filename) {
  MapData *loaded = malloc(sizeof(MapData));
  FILE *f = fopen(filename, "r");
  loaded->tileset_path = malloc(sizeof(char) * 100);
  fscanf(f, "%s", loaded->tileset_path);
  fscanf(f, "%d %d", &loaded->rows, &loaded->cols);

  // Read map from text file
  int **layout = malloc(loaded->rows * sizeof(int *));
  for (int i = 0; i < loaded->rows; i++) {
    layout[i] = malloc(loaded->cols * sizeof(int));
  }
  for (int i = 0; i < loaded->rows; i++) {
    for (int j = 0; j < loaded->cols; j++) {
      fscanf(f, "%d", &layout[i][j]);
    }
  }

  loaded->data = layout;
  // Init tiles
  loaded->tileset_data = tileset_init(loaded->tileset_path);
  fclose(f);
  return loaded;
}

MapData *init_level(Scene *scene, char *level, int* audio_stopper) {
  // Init SDL Stuff
  sdl_init(min, max);
  sdl_set_font("resources/comic_sans/normal.ttf", 20);
  sdl_set_zoom_range((Vector) {0.99, 5});

  ///////////////////////////
  //    Cannon Positions   //
  ///////////////////////////

  state.projectile_idx = 0;
  state.projectiles[0] = (display_ball) { .texture = "resources//graphics//projectiles//normal.png",
    .ball_num1 = 1000, .ball_num2 = 1000 };
  state.projectiles[1] = (display_ball) { .texture = "resources//graphics//projectiles//bouncy.png",
    .ball_num1 =   10, .ball_num2 =   10 };
  state.projectiles[2] = (display_ball) { .texture = "resources//graphics//projectiles//bullet_bill.png",
    .ball_num1 =   10, .ball_num2 =   10 };
  state.projectiles[3] = (display_ball) { .texture = "resources//graphics//projectiles//spike.png",
    .ball_num1 =   10, .ball_num2 =   10 };
  state.projectiles[4] = (display_ball) { .texture = "resources//graphics//projectiles//face.png",
    .ball_num1 =  100, .ball_num2 =   10 };

  state.projectile = state.projectiles[0];

  state.positions[0] = (cannon_positions){.filename = "resources//levels//bechtel.txt", .cannon1 = (Vector){min.x,10},
    .cannon2 = (Vector){max.x-30,10}};
  state.positions[1] = (cannon_positions){.filename = "resources//levels//first.txt", .cannon1 = (Vector){min.x + 86, min.y + 65},
    .cannon2 = (Vector){max.x - 120, min.y + 65}};
  state.positions[2]= (cannon_positions){.filename = "resources//levels//float.txt", .cannon1 = (Vector){min.x+100, -15},
    .cannon2 = (Vector){max.x-130, -15}};
  state.positions[3] = (cannon_positions) {.filename = "resources//levels//level1.txt", .cannon1 = (Vector){min.x + 108, min.y + 55},
    .cannon2 = (Vector){max.x - 140, min.y + 55}};
  state.positions[4] = (cannon_positions){.filename = "resources//levels//level2.txt", .cannon1 = (Vector){min.x + 108, min.y + 65},
    .cannon2 = (Vector){max.x - 140, min.y + 65}};
  state.positions[5] = (cannon_positions){.filename = "resources//levels//nether_test.txt", .cannon1 = (Vector){min.x+100,-65},
    .cannon2 = (Vector){max.x-130,-65}};
  state.positions[6] = (cannon_positions){.filename = "resources//levels//showcase.txt", .cannon1 = VEC_ZERO,
    .cannon2 = VEC_ZERO};
  state.positions[7] = (cannon_positions){.filename = "resources//levels//theshit.txt", .cannon1 = (Vector){min.x + 190, min.y + 70},
    .cannon2 = (Vector){max.x - 225, min.y + 70}};
  state.positions[8] = (cannon_positions){.filename = "resources//levels//level3.txt", .cannon1 = (Vector){min.x+200,-47},
    .cannon2 = (Vector){max.x-235,-47}};


  Vector b1 = VEC_ZERO;
  Vector b2 = VEC_ZERO;
  for (int i = 0; i < 9; i++) {
    cannon_positions temp = state.positions[i];
    if (strcmp(temp.filename, level) == 0) {
      b1 = temp.cannon1;
      b2 = temp.cannon2;
      break;
    }
  }

  ///////////////////////////
  //  End Cannon Positions //
  ///////////////////////////

  ///////////////////////////
  //     Bounding Wall     //
  ///////////////////////////

  Body* right_wall = body_init(polygon_rect((Vector){max.x + 2 * BALL_SIZE,max.y + 300},
    (Vector){max.x + 100 + 2 * BALL_SIZE, min.y - 300}), INT_MAX, (RGBColor){1,1,1});
  Body* left_wall = body_init(polygon_rect((Vector){min.x - 2 * BALL_SIZE,min.y - 300},
    (Vector){min.x - 100 - 2 * BALL_SIZE, max.y + 300}), INT_MAX, (RGBColor){1,1,1});
  Body *top_wall = body_init(polygon_rect((Vector){min.x,max.y + 2 * BALL_SIZE + 300},
    (Vector){max.x, max.y + 400 + 2 * BALL_SIZE}), INT_MAX, (RGBColor) {1,1,1});
  Body *bottom_wall = body_init(polygon_rect((Vector){min.x,min.y - 2 * BALL_SIZE},
    (Vector){max.x, min.y - 2 * BALL_SIZE - 100}), INT_MAX, (RGBColor) {1,1,1});

  // scene_add_body(scene, make_background(max, min));
  scene_add_body(scene, right_wall);
  scene_add_body(scene, left_wall);
  scene_add_body(scene, top_wall);
  scene_add_body(scene, bottom_wall);

  ///////////////////////////
  //   End Bounding Wall   //
  ///////////////////////////

  ///////////////////////////
  //    Background Stuff   //
  ///////////////////////////

  Body *back_boi = body_init(polygon_rect(max,min), 0, color_rand());
  body_set_size(back_boi, vec_subtract(max,min));
  body_set_texture(back_boi, sdl_load_image("resources/graphics/backgrounds/big_boi.png"));
  scene_add_scrolling_background(scene, back_boi, 0.5, VEC_ZERO, (Vector){-1, -1});

  Body *back_trees = body_init(polygon_rect(max,min), 0, color_rand());
  body_set_size(back_trees, vec_subtract(max,min));
  body_set_texture(back_trees, sdl_load_image("resources/graphics/backgrounds/trees.png"));
  scene_add_scrolling_background(scene, back_trees, 0.5, (Vector){0, -100}, (Vector){-1, -1});

  //////////////////////////
  // End Background Stuff //
  //////////////////////////

  MapData *map = load_map(level);

  int i, j;
  // Add in all of the tiles here
  for (i = 0; i < map->rows; i++) {
    for (j = 0; j < map->cols; j++) {
      int c = map->data[i][j];
      if (c == 0) {
        continue;
      }
      Body *block = make_block(BLOCK_SIZE, BLOCK_SIZE, 1);
      BodyInfo *info = BodyInfo_init_tile(map, c);
      block->info = (void *) info, block->freer = free;
      block->fill_color = (RGBColor) {(double)(c) / 30,  (double)(c) / 20, (double)(c) / 10};
      body_set_texture_id(block, c);
      body_set_centroid(block, (Vector) {j * BLOCK_SIZE + min.x, i * (-1 * BLOCK_SIZE) - min.y});
      scene_add_body(scene, block);
      body_set_size(block, (Vector){BLOCK_SIZE, BLOCK_SIZE});
    }
  }

  ///////////////////////////
  //      Cannon Stuff     //
  ///////////////////////////

  List *walls = list_init(4, NULL);
  list_add(walls, right_wall), list_add(walls, left_wall);
  list_add(walls, top_wall),   list_add(walls, bottom_wall);

  Body *barrel1 = add_cannon(scene, b1, true);
  Body *barrel2 = add_cannon(scene, b2, false);
  scene_add_body_type(scene, cannon_ball, walls, (void *) barrel1);
  scene_add_body_type(scene, cannon_ball, walls, (void *) barrel2);

  // create cannon firing and key handler
  fire_args *fa1 = malloc(sizeof(fire_args));
  fa1->scene = scene, fa1->walls = walls, fa1->data = (void *) barrel1;

  fire_args *fa2 = malloc(sizeof(fire_args));
  fa2->scene = scene, fa2->walls = walls, fa2->data = (void *) barrel2;

  List *key_info = list_init(2, free);
  list_add(key_info, (void *) fa1), list_add(key_info, (void *) fa2);
  sdl_on_key(on_key, (void *) key_info);

  List *mouse_info = list_init(3, NULL);
  list_add(mouse_info, (void *) barrel1), list_add(mouse_info, (void *) barrel2);
  list_add(mouse_info, (void *) audio_stopper);
  sdl_on_mouse(on_mouse, (void *) mouse_info);

  ///////////////////////////
  //    End Cannon Stuff   //
  ///////////////////////////

  ///////////////////////////
  //    Power Bar Stuff    //
  ///////////////////////////

  Body *power_ball = make_ball(2);
  BodyInfo_init(power_ball, 69);
  body_set_centroid(power_ball, (Vector){min.x+110, min.y+15});

  Body *power_ball2 = make_ball(2);
  BodyInfo_init(power_ball2, 69);
  body_set_centroid(power_ball2, (Vector){max.x-110, min.y+15});

  Body *power_rectangle = body_init(polygon_rect((Vector){min.x + 60, min.y + 13},(Vector){min.x + 160, min.y + 17}), 10, (RGBColor){0,1,0});
  BodyInfo_init(power_rectangle, 69);
  Body *power_rectangle2 = body_init(polygon_rect((Vector){max.x - 60, min.y + 13},(Vector){max.x - 160, min.y + 17}), 10, (RGBColor){0,1,0});
  BodyInfo_init(power_rectangle2, 69);

  Body *ball1 = make_ball(BALL_SIZE);
  state.proj_display1 = ball1;
  BodyInfo_init(state.proj_display1, 69);
  state.projectile = state.projectiles[state.projectile_idx];
  char* texture = get_display_ball_char(state.projectile);
  body_set_texture(state.proj_display1, sdl_load_image(texture));
  body_set_size(state.proj_display1,(Vector){2 * BALL_SIZE, 2 * BALL_SIZE});
  body_set_centroid(state.proj_display1, (Vector){min.x + 50, min.y + 13});

  Body *ball2 = make_ball(BALL_SIZE);
  state.proj_display2 = ball2;
  BodyInfo_init(state.proj_display2, 69);
  state.projectile = state.projectiles[state.projectile_idx];
  char* texture2 = get_display_ball_char(state.projectile);
  body_set_texture(state.proj_display2, sdl_load_image(texture2));
  body_set_size(state.proj_display2,(Vector){2 * BALL_SIZE, 2 * BALL_SIZE});
  body_set_centroid(state.proj_display2, (Vector){max.x - 50, min.y + 13});

  state.power_bar = power_rectangle;
  state.power_ball = power_ball;
  state.power_bar2 = power_rectangle2;
  state.power_ball2 = power_ball2;

  scene_add_body(scene, state.power_bar);
  scene_add_body(scene, state.power_ball);
  scene_add_body(scene, state.power_bar2);
  scene_add_body(scene, state.power_ball2);
  scene_add_body(scene, state.proj_display1);
  scene_add_body(scene, state.proj_display2);

  ///////////////////////////
  //  End Power Bar Stuff  //
  ///////////////////////////

  ///////////////////////////
  //       Wind Stuff      //
  ///////////////////////////

  state.gamma_string = malloc(sizeof(char)*50);
  sprintf(state.gamma_string, "Wind: %.0f mph", state.gamma);
  scene_add_text(scene, state.gamma_string, (RGBColor) {1,1,0}, (Vector) {max.x-130, max.y}, true, 31);

  state.ball_num_str1 = malloc(sizeof(char)*50);
  sprintf(state.ball_num_str1, "%d", 1000);
  scene_add_text(scene, state.ball_num_str1, (RGBColor) {1,1,0}, (Vector) {min.x, min.y+23}, true, 40);

  state.ball_num_str2 = malloc(sizeof(char)*50);
  sprintf(state.ball_num_str2, "%d", 1000);
  scene_add_text(scene, state.ball_num_str2, (RGBColor) {1,1,0}, (Vector) {max.x-45, min.y+23}, true, 50);

  ///////////////////////////
  //     End Wind Stuff    //
  ///////////////////////////

  // Init damage texture
  state.damage_texture = tileset_init("resources//textures//damage.txt");

  return map;
}

/////////////////////////
//    checkgame_over   //
/////////////////////////

void check_game_over(Scene *scene, Mix_Music *music) {
  if (state.game_over == 1) {
    play_music(music);
    scene_add_text(scene, "Player 1 wins!!!", (RGBColor) {1,1,0}, (Vector) {-50, max.y-40}, true, 70);
    state.locked = false;
    nu_send_str(state.conn, "OVER1");
  }
  else if (state.game_over == 2) {
    pause_music();
    play_music(music);
    scene_add_text(scene, "Player 2 wins!!!", (RGBColor) {1,1,0}, (Vector) {-50, max.y-40}, true, 70);
    state.locked = false;
    nu_send_str(state.conn, "OVER2");
  }
}


///////////////////////////
//     End Game Init     //
///////////////////////////

int main(int argc, char **argv) {
  // Init game state
  state.game_over = 0;
  state.mouse_state = false;
  state.turn = true;
  state.body_count = 0;
  state.locked = true;
  state.networked = true;
  state.status = argv[1];
  Mix_Music *end_game_music;

  ///////////////////////////
  //      Networking       //
  ///////////////////////////

  if (argc < 2 || argc > 5 || (strcmp(argv[1], "server") != 0 && strcmp(argv[1], "client") != 0
      && strcmp(argv[1], "local") != 0))
  {
    fprintf(stderr, "Usage.  There are three modes: server, client, and local.\n");
    fprintf(stderr, "Server Mode:\n");
    fprintf(stderr, "  %s server <server port>\n", argv[0]);
    fprintf(stderr, "Client Mode:\n");
    fprintf(stderr, "  %s client <server ip> <server port>\n", argv[0]);
    fprintf(stderr, "Local Mode:\n");
    fprintf(stderr, " %s local\n", argv[0]);
    exit(1);
  }

  if (strcmp(argv[1], "server") == 0) {
    printf("Waiting for a client to connect...\n");
    state.conn = nu_wait_client(atoi(argv[2]));
    printf("Client connected!\n");
  } else if (strcmp(argv[1], "client") == 0) {
    state.conn = nu_connect_server(argv[2], atoi(argv[3]));
    printf("Connected to server!\n");
  } else {
    state.networked = false;
  }

  if (state.conn < 0 && state.networked) {
    fprintf(stderr, "%s\n", strerror(errno));
    exit(1);
  }

  ///////////////////////////
  //     End Networking    //
  ///////////////////////////

  ///////////////////////////
  //       Map Stuff       //
  ///////////////////////////

  srand(time(0));
  Scene *scene = scene_init();
  double dt = 0;

  if (strcmp(argv[1], "server") == 0) {
    nu_send_str(state.conn, argv[3]);
  } 

  // will be used to store the channel of audio that's playing (fuse sound) and needs to be stopped
  int *audio_stopper = malloc(sizeof(int));
  *audio_stopper = -1;

  // Gets correct map from server if client
  char *mapname = strcmp(argv[1], "server") == 0 ? argv[3] : (strcmp(argv[1], "client") == 0 ?
    nu_try_read_str(state.conn) : argv[2]);
  MapData *map = init_level(scene, mapname, audio_stopper);

  // load sounds/music
  state.firing = load_audio_chunk("resources//audio//chunks//Boom.wav");
  state.fuse = load_audio_chunk("resources//audio//chunks//bomb_fuse.wav");
  state.music = load_music("resources//audio//music//skirmish_wav.wav");
  end_game_music = load_music("resources//audio//music//rick.wav");

  play_music(state.music);

  RGBColor text_col = (RGBColor) {0.7, 0.3, 0};
  if (strcmp(argv[1], "client") == 0) {
    scene_add_text(scene, "Client Mode", text_col, (Vector) {min.x, max.y}, true, 0);
    free(mapname);
  } else if (strcmp(argv[1], "server") == 0) {
    scene_add_text(scene, "Server Mode", text_col, (Vector) {min.x, max.y}, true, 0);
  } else {
    scene_add_text(scene, "Local Mode", text_col, (Vector) {min.x, max.y}, true, 0);
  }

  ///////////////////////////
  //     End Map Stuff     //
  ///////////////////////////

  ///////////////////////////
  //       Game Loop       //
  ///////////////////////////

  while (!sdl_is_done()) {
    // Read messages from other player's game
    if (state.networked) {
      char *remote = nu_try_read_str(state.conn);
      if (remote != NULL) {
        if (strcmp(remote, "QUIT") == 0) {
          free(remote);
          break;
        }
        else if (strcmp(remote, "SWITCHTURN") == 0) {
          state.projectile_idx = 0;
          update_projectiles();
          state.locked = false;
          parse_command(scene, remote, &state.turn);
        }
        else if (strcmp(remote, "CHANGE") == 0) {
          parse_command(scene, remote, &state.projectile_idx);
          update_projectiles();
        }
        else if (strcmp(remote, "OVER1") == 0 || strcmp(remote, "OVER2") == 0) {
          parse_command(scene, remote, &state.game_over);
        }
        else {
          parse_command(scene, remote, &state.body_count);
        }
        free(remote);
      }
    }

    ///////////////////////////
    //      Change Wind      //
    ///////////////////////////

    if (state.turn) {
      sprintf(state.gamma_string, "Wind: %.0f mph", state.gamma / 10);
      scene_change_text(scene, 31, state.gamma_string);

      if (body_get_centroid(state.power_ball).x >= min.x + 160) {
        body_set_velocity(state.power_ball, (Vector) {-POWERBALL_SPEED, 0});
      }
      else if (body_get_centroid(state.power_ball).x <= min.x + 60) {
        body_set_velocity(state.power_ball, (Vector) {POWERBALL_SPEED, 0});
      }
      scene_change_text(scene, 40, state.ball_num_str1);
    }
    else {
      sprintf(state.gamma_string, "Wind: %.0f mph", state.gamma / 10);
      scene_change_text(scene, 31, state.gamma_string);

      if (body_get_centroid(state.power_ball2).x <= max.x - 160) {
        body_set_velocity(state.power_ball2, (Vector) {POWERBALL_SPEED, 0});
      }
      else if (body_get_centroid(state.power_ball2).x >= max.x - 60) {
        body_set_velocity(state.power_ball2, (Vector) {-POWERBALL_SPEED, 0});
      }
      scene_change_text(scene, 50, state.ball_num_str2);
    }

    ///////////////////////////
    //    End Change Wind    //
    ///////////////////////////

    // Check if game over
    check_game_over(scene, end_game_music);

    // Update the screen
    sdl_update_scroll(dt);
    sdl_render_scene(scene, map->tileset_data);
    dt = time_since_last_tick();
    scene_tick(scene, dt);
  }

  ///////////////////////////
  //     End Game Loop     //
  ///////////////////////////

  // If done, send message to other game to close
  if (state.networked) {
    nu_send_str(state.conn, "QUIT");
    nu_close_connection(state.conn);
  }

  // Free everything
  for (int i = 0; i < map->rows; i++) {
    free(map->data[i]);
  }
  free(map->data);
  list_free(map->tileset_data);
  free(map->tileset_path);
  free(state.gamma_string);
  free(state.ball_num_str1);
  free(state.ball_num_str2);
  list_free(state.damage_texture);
  free(map);
  free(audio_stopper);
  Mix_FreeChunk(state.fuse);
  Mix_FreeChunk(state.firing);
  Mix_FreeMusic(state.music);
  Mix_FreeMusic(end_game_music);
  scene_free(scene);

  return 0;
}
