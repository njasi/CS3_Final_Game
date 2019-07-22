#ifndef __SCENE_H__
#define __SCENE_H__

#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <assert.h>
#include <string.h>
#include "body.h"
#include "list.h"

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*ForceCreator)(void *aux);

typedef struct {
  char *string;
  RGBColor color;
  Vector position;
  bool fixed;
  int id;
} Text;

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene {
  List *bodies;
  List *force_creators;
  List *force_arguments;
  List *force_freers;
  List *force_bodies;
  List *strings;
  List *body_packs;
  List *backgrounds;
} Scene;

typedef struct {
  Body *back;
  Vector repeat;
} Background;

typedef void (*BodyCreator)(Scene *scene, List *walls, void *data);

typedef struct {
  BodyCreator creator;
  Scene *scene;
  List *walls;
  void *data;
} BodyPack;

void text_free(void *t);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
Scene *scene_init(void);

/**
 * Releases memory allocated for a given scene and all its bodies.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(Scene *scene);

/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(Scene *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
Body *scene_get_body(Scene *scene, size_t index);

Body *scene_get_body_id(Scene *scene, int id);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(Scene *scene, Body *body);

/**
 * Adds a scrolling background to the scene
 * 
 * @param body the body that will be scrolling in the background.
 * @param scroll_speed the speed at which the body will scroll (.5 half speed etc)
 * @param center the coords of the starting centroid of the body.
 * @param repeat_times the tines that the bachground repeats (-1, -1, for forever)
 */
void scene_add_scrolling_background(Scene *scene, Body *back, double scroll_speed, Vector center, Vector repeat_times);

/**
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(Scene *scene, size_t index);

void scene_remove_body_id(Scene *scene, int id);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux,
                             FreeFunc freer);

void scene_add_bodies_force_creator(Scene *scene, List *bodies, ForceCreator
                                    forcer, void *aux, FreeFunc freer);

List *scene_strings(Scene *scene);

void scene_add_text(Scene *scene, char *string, RGBColor color, Vector position, bool fixed, int id);

void scene_change_text(Scene *scene, int id, char *new);

void scene_remove_text(Scene *scene, int id);

void scene_add_body_type(Scene *scene, BodyCreator creator, List *walls, void *data);

void scene_create_body_type(Scene *scene, int id, int type);

 /**
  * Executes a tick of a given scene over a small time interval.
  * This requires executing all the force creators
  * and then ticking each body (see body_tick()).
  *
  * @param scene a pointer to a scene returned from scene_init()
  * @param dt the time elapsed since the last tick, in seconds
  */
void scene_tick(Scene *scene, double dt);

#endif // #ifndef __SCENE_H__
