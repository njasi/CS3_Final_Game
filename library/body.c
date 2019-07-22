#include <limits.h>
#include "body.h"
#include "polygon.h"

Body *body_init(List *shape, double mass, RGBColor color) {
  Body *object = malloc(sizeof(Body));
  assert(object != NULL);
  object->forces = list_init(1, vec_free);
  object->impulses = list_init(1, vec_free);
  object->polygon = shape;
  object->mass = mass;
  object->fill_color = color;
  object->elasticity = 0;
  object->angular_speed = 0;
  object->angle = 0;
  object->velocity = VEC_ZERO;
  object->info = NULL;
  object->remove = false;
  object->colliding = false;
  object->texture_id = -1;
  object->texture = NULL;
  object->damage_overlay = NULL;
  object->size = VEC_ZERO;
  object->ss= 1.0;
  return object;
}

Body *body_init_with_info(List *shape, double mass, RGBColor color, void *info, FreeFunc info_freer) {
  Body *object = malloc(sizeof(Body));
  assert(object != NULL);
  object->forces = list_init(1, vec_free);
  object->impulses = list_init(1, vec_free);
  object->polygon = shape;
  object->mass = mass;
  object->fill_color = color;
  object->elasticity = 0;
  object->angular_speed = 0;
  object->angle = 0;
  object->velocity = VEC_ZERO;
  object->info = info;
  object->freer = info_freer;
  object->remove = false;
  object->colliding = false;
  object->texture_id = -1;
  object->texture = NULL;
  object->damage_overlay = NULL;
  object->size = VEC_ZERO;
  object->ss= 1.0;
  return object;
}

void body_set_id(Body *body, int id) {
  body->id = id;
}

void remove_flag(Body *body) {
  body->remove = true;
}

bool get_remove(Body *body) {
  return body->remove;
}

void *body_get_info(Body *body) {
  return body->info;
}

void body_set_texture_id(Body *body, int t) {
  body->texture_id = t;
}

void body_set_texture(Body *body, SDL_Texture *tex) {
  body->texture = tex;
}
void body_set_damage_overlay(Body *body, SDL_Texture *dam) {
  body->damage_overlay = dam;
}

void body_set_size(Body *body, Vector size){
  body->size = size;
}

void body_free(void *free_me) {
  if ((((Body *) free_me)->info) != NULL) {
    ((Body *) free_me)->freer(((Body *) free_me)->info);
  }
  list_free(((Body *) free_me)->polygon);
  list_free(((Body *) free_me)->forces);
  list_free(((Body *) free_me)->impulses);
  free(free_me);
}

List *body_get_shape(Body *body) {
  return body->polygon;
}

Vector body_get_centroid(Body *body) {
  return polygon_centroid(body->polygon);
}

Vector body_get_velocity(Body *body) {
  return body->velocity;
}

double body_get_mass(Body *body) {
  return body->mass;
}

RGBColor body_get_color(Body *body) {
  return body->fill_color;
}

void body_set_centroid(Body *body, Vector x) {
  polygon_translate(body->polygon,x);
}

void body_set_velocity(Body *body, Vector v) {
  body->velocity = v;
}

void body_set_rotation(Body *body, double angle) {
  polygon_rotate(body->polygon, angle - body->angle,
                 polygon_centroid(body->polygon));
  body->angle = angle;
}

void body_add_force(Body *body, Vector force) {
  Vector *to_add = (Vector *) malloc(sizeof(Vector));
  to_add->x = force.x, to_add->y = force.y;
  list_add(body->forces, to_add);
}

void body_add_impulse(Body *body, Vector impulse) {
  Vector *to_add = (Vector *) malloc(sizeof(Vector));
  to_add->x = impulse.x, to_add->y = impulse.y;
  list_add(body->impulses, to_add);
}

void body_tick(Body *body, double dt) {
  size_t i;
  double accel_x = 0, accel_y = 0;
  double impul_x = 0, impul_y = 0;
  Vector *force, *impulse;

  for (i = 0; i < list_size(body->forces); i++) {
    force = (Vector *) list_get(body->forces, i);
    accel_x += (force->x / body->mass) * dt;
    accel_y += (force->y / body->mass) * dt;
  }

  for (i = 0; i < list_size(body->impulses); i++) {
    impulse = (Vector *) list_get(body->impulses, i);
    impul_x += impulse->x / body->mass;
    impul_y += impulse->y / body->mass;
  }

  while (list_size(body->forces) != 0) {
    body->forces->freer(list_remove(body->forces, 0));
  }

  while (list_size(body->impulses) != 0) {
    body->impulses->freer(list_remove(body->impulses, 0));
  }

  body->velocity.x += accel_x + impul_x, body->velocity.y += accel_y + impul_y;
  double dist_x = body->velocity.x * dt, dist_y = body->velocity.y * dt;
  double rotation = body->angular_speed * dt;

  Vector translation = (Vector) { .x = dist_x, .y = dist_y };
  polygon_translate(body->polygon, translation);
  // polygon_rotate(body->polygon, rotation, centroid);
  body->angle += rotation;
}

bool on_line(line l, Vector p) {
  if (p.x <= fmax(l.a.x, l.b.x) && p.x <= fmin(l.a.x, l.b.x) &&
      (p.y <= fmax(l.a.y, l.b.y) && p.y <= fmin(l.a.y, l.b.y))) {
      return true;
    }
  return false;
}

int direction(Vector a, Vector b, Vector c) {
  int val = (b.y - a.y) * (c.x - b.x) - (b.x - a.x) * (c.y - b.y);
  if (val == 0) {
    return 0; // on the same line
  }
  else if (val < 0) {
    return 2; // counterclockwise direction
  }
  return 1; // clockwise direction
}

bool line_intersect(line l1, line l2) {
  // four directions for two lines and points of other line
  int dir1 = direction(l1.a, l1.b, l2.a);
  int dir2 = direction(l1.a, l1.b, l2.b);
  int dir3 = direction(l2.a, l2.b, l1.a);
  int dir4 = direction(l2.a, l2.b, l1.b);

  if (dir1 != dir2 && dir3 != dir4) {
    return true; // they intersect
  }
  if (dir1 == 0 && on_line(l1, l2.a)) { // when b of l2 is on l1
    return true;
  }
  if (dir2 == 0 && on_line(l1, l2.b)) { // when a of l2 is on l1
    return true;
  }
  if (dir3 == 0 && on_line(l2, l1.a)) { // when b of l1 is on l2
    return true;
  }
  if (dir4 == 0 && on_line(l2, l1.b)) { // when a of l1 is on l2
    return true;
  }
  return false;
}

List *get_edges(List *shape) {
  size_t i;
  Vector t, s;
  List *edges = list_init(1, vec_free);
  for (i = 0; i < list_size(shape); i++) {
    t = *((Vector *) list_get(shape, i));
    s = *((Vector *) list_get(shape, (i+1)%list_size(shape)));
    Vector diff = vec_subtract(s, t);
    Vector *edge = (Vector *) malloc(sizeof(Vector));
    assert(edge != NULL);
    edge->x = diff.x, edge->y = diff.y;
    list_add(edges, edge);
  }
  return edges;
}

List *get_axes(List *edges) {
  size_t i;
  Vector e;
  List *axes = list_init(1, vec_free);
  for (i = 0; i < list_size(edges); i++) {
    e = *((Vector *) list_get(edges, i));
    Vector *axis = (Vector *) malloc(sizeof(Vector));
    assert(axis != NULL);
    axis->x = e.y, axis->y = -1 * e.x;
    list_add(axes, axis);
  }
  return axes;
}

bool body_collide(Body *body1, Body *body2) {
  /*
   * Separating Axis Theorem
   * 1. Compute edge vectors of both shapes and put into list
   * 2. Compute perpendicular axes to each edge and put into list ({x, y} -> {y, -x})
   * 3. Compute dot product of each vertex vector with each axis' unit vector and put into list
   * 4. Keep the minimum and maximum dot product for each shape
   * 5. If the maximum of one shape is less than the maximum of the other shape, return false
   * 6. Check this for each axis, and if does not return false, return true
   */

   size_t i, j;
   Vector v, u;
   double d, min1, max1, min2, max2;
   if (in_bounding_box(body1, body2) && !body1->colliding && !body2->colliding) {
     List *shape1 = body_get_shape(body1), *shape2 = body_get_shape(body2);
     List *edges1 = get_edges(shape1), *edges2 = get_edges(shape2);
     List *edges = list_append(edges1, edges2);
     List *axes = get_axes(edges);
     for (i = 0; i < list_size(axes); i++) {
       min1 = INT_MAX, max1 = 0, min2 = INT_MAX, max2 = 0;
       u = vec_unit(*((Vector *) list_get(axes, i)));
       for (j = 0; j < list_size(shape1); j++) {
         v = *((Vector *) list_get(shape1, j));
         d = vec_dot(v, u);
         if (d < min1) {
           min1 = d;
         }
         if (d > max1) {
           max1 = d;
         }
       }
       for (j = 0; j < list_size(shape2); j++) {
         v = *((Vector *) list_get(shape2, j));
         d = vec_dot(v, u);
         if (d < min2) {
           min2 = d;
         }
         if (d > max2) {
           max2 = d;
         }
       }
       if (max2 < min1 || max1 < min2) {
         list_free(edges1), list_free(edges2);
         free(edges->data), free(edges);
         list_free(axes);
         body1->colliding = false, body2->colliding = false;
         return false;
       }
     }
     list_free(edges1), list_free(edges2);
     free(edges->data), free(edges);
     list_free(axes);
     body1->colliding = true, body2->colliding = true;
     return true;
   }
   body1->colliding = false, body2->colliding = false;
   return false;

  /*
  // Vertex Containment Method
  size_t i, j;
  Vector e, v, s, p;
  List *shape1, *shape2, *edges1;
  bool inside;
  if (in_bounding_box(body1, body2)) {
    shape1 = body_get_shape(body1), shape2 = body_get_shape(body2);
    edges1 = get_edges(shape1);
    for (i = 0; i < list_size(shape2); i++) {
      inside = true;
      s = *((Vector *) list_get(shape2, i));
      for (j = 0; j < list_size(edges1); j++) {
        e = *((Vector *) list_get(edges1, j));
        v = *((Vector *) list_get(shape1, j));
        p = vec_subtract(s, v);
        if (vec_cross(e, p) < 0) {
          inside = false;
        }
      }
      if (inside) {
        list_free(edges1);
        return true;
      }
    }
    list_free(edges1);
  }
  return false;
  */
}

Vector get_collision_axis(Body *body1, Body *body2) {
  size_t i, j;
  Vector v, u, c;
  double d, min1, max1, min2, max2, diff = INT_MAX;
  List *shape1 = body_get_shape(body1), *shape2 = body_get_shape(body2);
  List *edges1 = get_edges(shape1), *edges2 = get_edges(shape2);
  List *edges = list_append(edges1, edges2);
  List *axes = get_axes(edges);
  for (i = 0; i < list_size(axes); i++) {
       min1 = INT_MAX, max1 = 0, min2 = INT_MAX, max2 = 0;
       u = vec_unit(*((Vector *) list_get(axes, i)));
       for (j = 0; j < list_size(shape1); j++) {
         v = *((Vector *) list_get(shape1, j));
         d = vec_dot(v, u);
         if (d < min1) {
           min1 = d;
         }
         if (d > max1) {
           max1 = d;
         }
       }
       for (j = 0; j < list_size(shape2); j++) {
         v = *((Vector *) list_get(shape2, j));
         d = vec_dot(v, u);
         if (d < min2) {
           min2 = d;
         }
         if (d > max2) {
           max2 = d;
         }
       }
       if (fabs(min2 - max1) < diff) {
         diff = fabs(min2 - max1);
         c = u;
       }
       if (fabs(min1 - max2) < diff) {
         diff = fabs(min1 - max2);
         c = u;
       }
     }
     list_free(edges1), list_free(edges2);
     free(edges->data), free(edges);
     list_free(axes);
     return c;
}

bool boundary_collide(List *object, double boundary, size_t type) {
  size_t i = 0;
  switch(type) {
    case 0: // y below
      for (; i < list_size(object); i++)
        if (((Vector *) list_get(object, i))->y < boundary)
          return true;
      break;
    case 1: // y above
      for (; i < list_size(object); i++)
        if (((Vector *) list_get(object, i))->y > boundary)
          return true;
      break;
    case 2: // x to the right
      for(; i < list_size(object); i++)
        if (((Vector *) list_get(object, i))->x > boundary)
          return true;
      break;
    case 3: // x to the left
      for (; i < list_size(object); i++)
        if (((Vector *) list_get(object, i))->x < boundary)
          return true;
      break;
  }
  return false;
}

bool in_bounding_box(Body *one, Body *two){
  size_t i;
  double dist;
  double bound_one = 0;
  double bound_two = 0;
  List *sone = body_get_shape(one);
  List *stwo = body_get_shape(two);
  Vector cone = body_get_centroid(one);
  Vector ctwo = body_get_centroid(two);
  for(i = 0, dist = 0; i < list_size(sone); i++) {
    dist = vec_distance(*(Vector *) list_get(sone, i), cone);
    if(dist > bound_one){
      bound_one = dist;
    }
  }
  for(i = 0, dist = 0; i < list_size(stwo); i++) {
    dist = vec_distance(*(Vector *) list_get(stwo, i), ctwo);
    if(dist > bound_two) {
      bound_two = dist;
    }
  }
  return vec_distance(cone, ctwo) < (bound_one + bound_two);
}
