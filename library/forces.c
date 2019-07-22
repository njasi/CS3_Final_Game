#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <math.h>
#include <limits.h>
#include "forces.h"
#include "collision.h"

#define DISTANCE_THRESHOLD 10

typedef struct {
  Scene *scene;
  double k;
  Body *body1;
  Body *body2;
  Vector axis;
} force_args;

typedef struct {
  CollisionHandler handler;
  Body *body1;
  Body *body2;
  Vector axis;
  void *aux;
} collision_args;

void physics_free(void *c) {
  collision_args *ca = (collision_args *) c;
  free(ca->aux);
  free(ca);
}

force_args *gravity_init(Scene *scene, double G, Body *body1, Body *body2) {
  force_args *ga = (force_args *) malloc(sizeof(force_args));
  assert(ga != NULL);
  ga->scene = scene, ga->k = G, ga->body1 = body1, ga->body2 = body2;
  return ga;
}

force_args *spring_init(Scene *scene, double k, Body *body1, Body *body2) {
  force_args *sa = (force_args *) malloc(sizeof(force_args));
  assert(sa != NULL);
  sa->scene = scene, sa->k = k, sa->body1 = body1, sa->body2 = body2;
  return sa;
}

force_args *drag_init(Scene *scene, double gamma, Body *body) {
  force_args *da = (force_args *) malloc(sizeof(force_args));
  assert(da != NULL);
  da->scene = scene, da->k = gamma, da->body1 = body;
  return da;
}


force_args *flat_earth_init(Scene *scene, double g, Body *body) {
  force_args *fa = (force_args *) malloc(sizeof(force_args));
  assert(fa != NULL);
  fa->scene = scene, fa->k = g, fa->body1 = body;
  return fa;
}

force_args *dest_collision_init(Scene *scene, Body *body1, Body *body2) {
  force_args *ca = (force_args *) malloc(sizeof(force_args));
  assert(ca != NULL);
  ca->scene = scene, ca->body1 = body1, ca->body2 = body2;
  return ca;
}

force_args *phys_collision_init(Scene *scene, Body *body1, Body *body2, Vector axis, double elas) {
  force_args *pa = (force_args *) malloc(sizeof(force_args));
  assert(pa != NULL);
  pa->scene = scene, pa->body1 = body1, pa->body2 = body2, pa->axis = axis, pa->k = elas;
  return pa;
}

collision_args *collision_init(CollisionHandler handler, Body *body1, Body *body2,
  Vector axis, void *aux)
  {
    collision_args *ca = (collision_args *) malloc(sizeof(collision_args));
    ca->handler = handler;
    ca->body1 = body1, ca->body2 = body2;
    ca->axis = axis;
    ca->aux = aux;
    return ca;
  }

double gravity_force(double G, double m1, double m2, double dist) {
  return (G * m1 * m2) / pow(dist, 2);
}

double spring_force(double k, double dist) {
  return k * dist;
}

Vector drag_force_cannon(double gamma, Body *body) {
  return (Vector){gamma + body_get_velocity(body).x, 0};
}

Vector drag_force(double gamma, Body *body) {
  return vec_multiply(gamma, body_get_velocity(body));
}


double get_dist(Body *body1, Body *body2) {
  Vector cen1 = body_get_centroid(body1);
  Vector cen2 = body_get_centroid(body2);
  return sqrt(pow(cen2.x - cen1.x, 2) + pow(cen2.y - cen1.y, 2));
}

Vector get_force(double f_mag, Vector cen1, Vector cen2, double dist) {
  return (Vector) {f_mag * ((cen2.x - cen1.x) / dist),
    f_mag * ((cen2.y - cen1.y) / dist)};
}

void gravity_wrapper(void *g) {
  force_args *ga = (force_args *) g;
  Vector cen1 = body_get_centroid(ga->body1);
  Vector cen2 = body_get_centroid(ga->body2);
  double dist = get_dist(ga->body1, ga->body2);
  double f_mag = gravity_force(ga->k, body_get_mass(ga->body1),
    body_get_mass(ga->body2), dist > DISTANCE_THRESHOLD ? dist : INT_MAX);

  Vector force = get_force(f_mag, cen1, cen2, dist);
  body_add_force(ga->body1, force);
  body_add_force(ga->body2, vec_negate(force));
}

void spring_wrapper(void *s) {
  force_args *sa = (force_args *) s;
  Vector cen1 = body_get_centroid(sa->body1);
  Vector cen2 = body_get_centroid(sa->body2);
  double dist = get_dist(sa->body1, sa->body2);
  double f_mag = spring_force(sa->k, dist);
  Vector force = get_force(f_mag, cen1, cen2, dist);
  body_add_force(sa->body1, force);
  body_add_force(sa->body2, vec_negate(force));
}

void drag_wrapper(void *d) {
  force_args *da = (force_args *) d;
  Vector force = drag_force(da->k, da->body1);
  body_add_force(da->body1, vec_negate(force));
}

void flat_earth_wrapper(void *f) {
  force_args *fa = (force_args *) f;
  Vector force = (Vector) {0, body_get_mass(fa->body1) * fa->k};
  body_add_force(fa->body1, vec_negate(force));
}

void drag_force_cannon_wrapper(void *f) {
  force_args *fa = (force_args *) f;
  Vector force = drag_force_cannon(fa->k, fa->body1);
  body_add_force(fa->body1, vec_negate(force));
}

void destruction_wrapper(void *c) {
  force_args *ca = (force_args *) c;
  if (body_collide(ca->body1, ca->body2)) {
    remove_flag(ca->body1);
    remove_flag(ca->body2);
  }
}

void physics_wrapper(Body *body1, Body *body2, Vector axis, void *aux) {
  double mass1 = body_get_mass(body1), mass2 = body_get_mass(body2), mass, impulse;
  Vector ax = axis;
  ax = get_collision_axis(body1, body2);
  // Calculates mass: if infinity, uses other mass; otherwise uses reduced mass
  mass = mass1 == INT_MAX ? mass2 : (mass2 == INT_MAX ? mass1 : (mass1 * mass2) / (mass1 + mass2));
  impulse = mass * (1 + *((double *) aux)) * (vec_dot(body_get_velocity(body2), ax)
   - vec_dot(body_get_velocity(body1), ax));
  if (mass1 != INT_MAX) body_add_impulse(body1, vec_multiply(impulse, ax));
  if (mass2 != INT_MAX) body_add_impulse(body2, vec_negate(vec_multiply(impulse, ax)));
}

void collision_wrapper(void *c) {
  collision_args *ca = (collision_args *) c;
  Vector axis;
  if (ca->body1->colliding) {
    polygon_translate(ca->body1->polygon, (Vector){body_get_velocity(ca->body1).x < 0 ? 5 : -5, 0});
    ca->body1->colliding = false;
    ca->body2->colliding = false;
    return;
  }
  if (body_collide(ca->body1, ca->body2)) {
    axis = get_collision_axis(ca->body1, ca->body2);
    ca->handler(ca->body1, ca->body2, axis, ca->aux);
  }
}

void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2) {
  force_args *g_args = gravity_init(scene, G, body1, body2);
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1), list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, bodies, gravity_wrapper, (void *) g_args, NULL);
}

void create_spring(Scene *scene, double k, Body *body1, Body *body2) {
  force_args *s_args = spring_init(scene, k, body1, body2);
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1), list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, bodies, spring_wrapper, (void *) s_args, NULL);
}

void create_drag(Scene *scene, double gamma, Body *body) {
  force_args *d_args = drag_init(scene, gamma, body);
  List *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, bodies, drag_wrapper, (void *) d_args, NULL);
}

void create_flat_earth(Scene *scene, double g, Body *body) {
  force_args *f_args = flat_earth_init(scene, g, body);
  List *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, bodies, flat_earth_wrapper, (void *) f_args, NULL);
}

void create_drag_force_cannon(Scene *scene, double g, Body *body ){
  force_args *f_args = drag_init(scene, g, body);
  List *bodies = list_init(1, NULL);
  list_add(bodies, body);
  scene_add_bodies_force_creator(scene, bodies, drag_force_cannon_wrapper, (void *) f_args, NULL);
}

void create_destructive_collision(Scene *scene, Body *body1, Body *body2) {
  force_args *c_args = dest_collision_init(scene, body1, body2);
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1), list_add(bodies, body2);
  scene_add_bodies_force_creator(scene, bodies, destruction_wrapper, (void *) c_args, NULL);
}

void create_collision(Scene *scene, Body *body1, Body *body2, CollisionHandler handler, void *aux, FreeFunc freer) {
  List *bodies = list_init(2, NULL);
  list_add(bodies, body1), list_add(bodies, body2);
  collision_args *ca = collision_init(handler, body1, body2, VEC_ZERO, aux);
  scene_add_bodies_force_creator(scene, bodies, collision_wrapper, ca, freer);
}

void create_physics_collision(Scene *scene, double elasticity, Body *body1, Body *body2) {
  double *elas = (double *) malloc(sizeof(double));
  *elas = elasticity;
  create_collision(scene, body1, body2, physics_wrapper, elas, physics_free);
}
