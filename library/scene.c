#include "scene.h"

void text_free(void *t) {
  Text *text = (Text *) t;
  free(text->string);
  free(text);
}

void background_free(void *to_free){
  Background *boi = (Background *) to_free;
  body_free((void *) (boi->back));
  free(boi);
}

Scene *scene_init(void) {
  Scene *scene = (Scene *) malloc(sizeof(Scene));
  assert(scene != NULL);
  scene->bodies = list_init(1, body_free);
  scene->force_creators = list_init(1, NULL);
  scene->force_arguments = list_init(1, free);
  scene->force_freers = list_init(1, NULL);
  scene->force_bodies = list_init(1, NULL);
  scene->strings = list_init(1, free);
  scene->body_packs = list_init(1, free);
  scene->backgrounds = list_init(1, background_free);
  return scene;
}

void scene_free(Scene *scene) {
  size_t i;
  FreeFunc freer;
  list_free(scene->bodies);
  list_free(scene->force_creators);
  for (i = 0; i < list_size(scene->force_arguments); i++) {
    freer = list_get(scene->force_freers, i);
    freer(list_remove(scene->force_arguments, i));
    i--;
  }
  list_free(scene->force_arguments);
  list_free(scene->force_freers);
  for (i = 0; i < list_size(scene->force_bodies); i++) {
    list_free(list_get(scene->force_bodies, i));
  }
  list_free(scene->force_bodies);
  list_free(scene->strings);
  list_free(scene->body_packs);
  list_free(scene->backgrounds);
  free(scene);
}

size_t scene_bodies(Scene *scene) {
  return list_size(scene->bodies);
}

Body *scene_get_body(Scene *scene, size_t index) {
  return (Body *) list_get(scene->bodies, index);
}

Body *scene_get_body_id(Scene *scene, int id) {
  size_t i;
  for (i = 0; i < scene_bodies(scene); i++) {
    if (scene_get_body(scene, i)->id == id) {
      return scene_get_body(scene, i);
    }
  }
  return NULL;
}

void scene_add_body(Scene *scene, Body *body) {
  list_add(scene->bodies, (void *) body);
}

void scene_add_scrolling_background(Scene *scene, Body *back, double scroll_speed, Vector center, Vector repeat_times) {
  body_set_centroid(back, center);
  Background *back_temp = malloc(sizeof(Background));
  if(back_temp == NULL) { // malloc failed
    printf("mallocing the background failed");
    exit(1);
  }
  back->ss = scroll_speed;
  back_temp->back = back;
  back_temp->repeat = repeat_times; // error here... why?
  list_add(scene->backgrounds, (void *) back_temp);
}

void scene_remove_body(Scene *scene, size_t index) {
  void *removed = list_remove(scene->bodies, index);
  body_free(removed);
}

void scene_remove_body_id(Scene *scene, int id) {
  size_t i;
  for (i = 0; i < scene_bodies(scene); i++) {
    if (scene_get_body(scene, i)->id == id) {
      scene_remove_body(scene, i);
    }
  }
}

void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux,
                             FreeFunc freer)
{
  scene_add_bodies_force_creator(scene, NULL, forcer, aux, freer);
}

void scene_add_bodies_force_creator(Scene *scene, List *bodies, ForceCreator
                                    forcer, void *aux, FreeFunc freer)
{
  list_add(scene->force_creators, forcer);
  list_add(scene->force_arguments, aux);
  if (freer == NULL) {
    list_add(scene->force_freers, free);
  } else {
    list_add(scene->force_freers, freer);
  }
  if (bodies != NULL) {
    list_add(scene->force_bodies, bodies);
  }
}

List *scene_strings(Scene *scene) {
  return scene->strings;
}

void scene_add_text(Scene *scene, char *string, RGBColor color, Vector position, bool fixed, int id) {
  Text *text = (Text *) malloc(sizeof(Text));
  // text->string = malloc(strlen(string) * sizeof(char));
  text->string = string;
  text->color = color;
  text->position = position;
  text->fixed = fixed;
  text->id = id;
  list_add(scene->strings, text);
}

void scene_change_text(Scene *scene, int id, char *new) {
  size_t i;
  for (i = 0; i < list_size(scene->strings); i++) {
    Text *text = (Text *) list_get(scene->strings, i);
    if (text->id == id) {
      //free(text->string);
      text->string = new;
    }
  }
}

void scene_remove_text(Scene *scene, int id) {
  size_t i;
  for (i = 0; i < list_size(scene->strings); i++) {
    Text *text = (Text *) list_get(scene->strings, i);
    if (text->id == id) {
      text_free(list_remove(scene->strings, i));
    }
  }
}

void scene_add_body_type(Scene *scene, BodyCreator creator, List *walls, void *data) {
  BodyPack *pack = (BodyPack *) malloc(sizeof(BodyPack));
  pack->creator = creator;
  pack->scene = scene;
  pack->walls = walls;
  pack->data = data;
  list_add(scene->body_packs, pack);
}

void scene_create_body_type(Scene *scene, int id, int type) {
  BodyPack *pack = (BodyPack *) list_get(scene->body_packs, (size_t) type);
  pack->creator(pack->scene, pack->walls, pack->data);
}

void scene_tick(Scene *scene, double dt) {
  size_t i, j, k;
  ForceCreator forcer;
  FreeFunc freer;
  for (i = 0; i < list_size(scene->force_creators); i++) {
    forcer = list_get(scene->force_creators, i);
    forcer(list_get(scene->force_arguments, i));
  }
  for (i = 0; i < list_size(scene->bodies); i++) {
    body_tick((Body *) list_get(scene->bodies, (int) i), dt);
  }
  for (i = 0; i < list_size(scene->bodies); i++) { // Loop through all the bodies
    if (get_remove(list_get(scene->bodies, i))) { // Check if body is flagged for removal
      for (j = 0; j < list_size(scene->force_arguments); j++) { // Loop through all force arguments
        List *list = list_get(scene->force_bodies, j);
        for (k = 0; k < list_size(list); k++) { // Loop through relevant bodies of force arguments
          if (list_get(list, k) == list_get(scene->bodies, i)) { // If body is present, force info
            freer = list_get(scene->force_freers, j);
            freer(list_remove(scene->force_arguments, j));
            list_remove(scene->force_freers, j);
            list_remove(scene->force_creators, j);
            list_free(list_remove(scene->force_bodies, j));
            j--;
            break;
          }
        }
      }
      scene_remove_body(scene, i); // Remove body
      i--;
    }
  }
}
