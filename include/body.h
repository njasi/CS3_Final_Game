#ifndef __BODY_H__
#define __BODY_H__

#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <assert.h>
#include <math.h>
#include <stdio.h>
#include <SDL2/SDL_render.h>
#include "color.h"
#include "list.h"
#include "polygon.h"

/**
 * A rigid body constrained to the plane.
 * Implemented as a polygon with uniform density.
 * Bodies can accumulate forces and impulses during each tick.
 * Angular physics (i.e. torques) are not currently implemented.
 */

 typedef struct {
   int type;
   double time_since_last_shot;
 } ship_data;

 typedef struct _body {
   List *polygon;
   List *forces;
   List *impulses;
   Vector velocity;
   Vector size;
   double elasticity, angular_speed, mass, angle;
   RGBColor fill_color;
   void *info;
   FreeFunc freer;
   bool remove;
   bool colliding;
   int id;
   int texture_id;
   SDL_Texture *texture;
   SDL_Texture *damage_overlay; // show breaking
/*
   ((info_strct*)body->info)->curr,max
   body->damage_overlay = tileset_get(damages, division stuff)    /// tileset_get_texture(tileset, diunhdiwhdw);

    */
   double ss;
 } Body;


 typedef struct _line {
   Vector a, b;
 } line;

/**
 * Allocates memory for a body with the given parameters.
 * The body is initially at rest.
 * Asserts that the required memory is allocated.
 *
 * @param shape a list of vectors describing the initial shape of the body
 * @param mass the mass of the body (if INFINITY, stops the body from moving).
 * @param color the color of the body, used to draw it on the screen
 * @return a pointer to the newly allocated body
 */
Body *body_init(List *shape, double mass, RGBColor color);

Body *body_init_with_info(List *shape, double mass, RGBColor color, void *info, FreeFunc info_freer);

void body_set_id(Body *body, int id);

void remove_flag(Body *body);

bool get_remove(Body *body);

void *body_get_info(Body *body);

void body_set_texture(Body *body, SDL_Texture *tex);

void body_set_damage_overlay(Body *body, SDL_Texture *dam);

void body_set_texture_id(Body *body, int t);

void body_set_size(Body *body, Vector size);

/**
 * Releases the memory allocated for a body.
 *
 * @param body a pointer to a body returned from body_init()
 */
void body_free(void *body);

/**
 * Gets the current shape of a body.
 * Returns a newly allocated vector list, which must be list_free()d.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the polygon describing the body's current position
 */
List *body_get_shape(Body *body);

/**
 * Gets the current center of mass of a body.
 * While this could be calculated with polygon_centroid(), that becomes too slow
 * when this function is called thousands of times every tick.
 * Instead, the body should store its current centroid.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's center of mass
 */
Vector body_get_centroid(Body *body);

/**
 * Gets the current velocity of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's velocity vector
 */
Vector body_get_velocity(Body *body);

/**
 * Gets the mass of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the mass passed to body_init(), which must be greater than 0
 */
double body_get_mass(Body *body);

/**
 * Gets the display color of a body.
 *
 * @param body a pointer to a body returned from body_init()
 * @return the body's color, as an (R, G, B) tuple
 */
RGBColor body_get_color(Body *body);

/**
 * Translates a body to a new position.
 * The position is specified by the position of the body's center of mass.
 *
 * @param body a pointer to a body returned from body_init()
 * @param x the body's new centroid
 */
void body_set_centroid(Body *body, Vector x);

/**
 * Changes a body's velocity (the time-derivative of its position).
 *
 * @param body a pointer to a body returned from body_init()
 * @param v the body's new velocity
 */
void body_set_velocity(Body *body, Vector v);

/**
 * Changes a body's orientation in the plane.
 * The body is rotated about its center of mass.
 * Note that the angle is *absolute*, not relative to the current orientation.
 *
 * @param body a pointer to a body returned from body_init()
 * @param angle the body's new angle in radians. Positive is counterclockwise.
 */
void body_set_rotation(Body *body, double angle);

/**
 * Applies a force to a body over the current tick.
 * If multiple forces are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param force the force vector to apply
 */
void body_add_force(Body *body, Vector force);

/**
 * Applies an impulse to a body.
 * An impulse causes an instantaneous change in velocity,
 * which is useful for modeling collisions.
 * If multiple impulses are applied in the same tick, they should be added.
 * Should not change the body's position or velocity; see body_tick().
 *
 * @param body a pointer to a body returned from body_init()
 * @param impulse the impulse vector to apply
 */
void body_add_impulse(Body *body, Vector impulse);

/**
 * Updates the body after a given time interval has elapsed.
 * Sets acceleration and velocity according to the forces and impulses
 * applied to the body during the tick.
 * The body should be translated at the *average* of the velocities before
 * and after the tick.
 * Resets the forces and impulses accumulated on the body.
 *
 * @param body the body to tick
 * @param dt the number of seconds elapsed since the last tick
 */
void body_tick(Body *body, double dt);

/**
* Checks if a point (vector) is on a line (struct containing two vectors)
*
* @param l the line that we are checking
* @param p the point we want to check
*/
bool on_line(line l, Vector p);

/**
* Checks the linearity of three points
*
* @param a a point
* @param b b point
* @param c c point
*/
int direction(Vector a, Vector b, Vector c);

/**
* Checks if two line segments collide
*
* @param l1 line one
* @param l2 line two
*/
bool line_intersect(line l1, line l2);

/**
 * Checks if the polygon one collides with pollygon 2
 *
 * @param one VectorList to compare
 * @param two VectorList to compare
 */
bool body_collide(Body *body1, Body *body2);

Vector get_collision_axis(Body *body1, Body *body2);

/**
 * Checks if the polygon one collides with a boundary
 *
 * @param one VectorList to compare
 * @param boundary the boundary value
 * @param type the type of boundary (0 = y boundary below,1=yabove,2=xright,3=xleft)
 */

bool boundary_collide(List *object, double boundary, size_t type);

bool in_bounding_box(Body *one, Body *two);

#endif // #ifndef __BODY_H__
