// TODO: IMPLEMENT YOUR TESTS IN THIS FILE
#include "forces.h"
#include "test_util.h"
#include "body.h"
#include "scene.h"
#include "vector.h"
#include "stdio.h"
#include "color.h"
#include "polygon.h"

#include <stdbool.h>
#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <limits.h>

int vec_diff(Vector a, Vector b){
  return (a.x - b.x) + (a.y -b.y);
}

List *make_shape() {
    List *shape = list_init(4, free);
    Vector *v = malloc(sizeof(*v));
    *v = (Vector) {-1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, -1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {+1, +1};
    list_add(shape, v);
    v = malloc(sizeof(*v));
    *v = (Vector) {-1, +1};
    list_add(shape, v);
    return shape;
}

double gravity_potential(double G, Body *body1, Body *body2) {
    Vector r = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    return -G * body_get_mass(body1) * body_get_mass(body2) / sqrt(vec_dot(r, r));
}
double kinetic_energy(Body *body) {
    Vector v = body_get_velocity(body);
    return body_get_mass(body) * vec_dot(v, v) / 2;
}

void test_newton_third_law() {
  const double M1 = 4.5, M2 = 7.3;
  const double G = 1e3;
  const double DT = 9e-7;
  const int STEPS = 1000000;
  Scene *scene = scene_init();
  Body *mass1 = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
  scene_add_body(scene, mass1);
  Body *mass2 = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
  body_set_centroid(mass2, (Vector) {10, 20});
  scene_add_body(scene, mass2);
  create_newtonian_gravity(scene, G, mass1, mass2);
  double initial_energy = gravity_potential(G, mass1, mass2);
  for (int i = 0; i < STEPS; i++) {
    assert(body_get_centroid(mass1).x < body_get_centroid(mass2).x); //  ^ same
    double energy = gravity_potential(G, mass1, mass2) +
        kinetic_energy(mass1) + kinetic_energy(mass2);
    assert(within(1e-5, energy/initial_energy, 1));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_period() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 5;
  const int STEPS = 1000000;
  double time = 0;
  Scene *scene = scene_init();
  Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass, (Vector) {A, 0});
  scene_add_body(scene, mass);
  Body *anchor = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  for (int i = 0; i < STEPS; i++) {
    if (vec_diff(body_get_centroid(mass), (Vector) {A, 0}) == 0 && i != 0) {
      assert(within(1e-5, time / (2 * M_PI * sqrt(M/K)), 1));
      time = 0;
    }
      time += DT;
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

double spring_potential(Body *body, double k){
  Vector x = body_get_centroid(body);
  return k * vec_dot(x, x) / 2;
}

void test_spring_energy_conservation() {
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  Scene *scene = scene_init();
  Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass, (Vector) {A, 0});
  scene_add_body(scene, mass);
  Body *anchor = body_init(make_shape(), INT_MAX, (RGBColor) {0, 0, 0});
  scene_add_body(scene, anchor);
  create_spring(scene, K, mass, anchor);
  double initial_energy = spring_potential(mass, K);
  for (int i = 0; i < STEPS; i++) {
    double energy = kinetic_energy(mass) + spring_potential(mass, K);
    assert(within(1e-5, energy, initial_energy));
    scene_tick(scene, DT);
  }
  scene_free(scene);
}


/* group 01 */
void test_momentum_conservation() {
    const double M1 = 10.3, M2 = 29.2;
    const double G = 1e3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass1 = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
    scene_add_body(scene, mass1);
    Body *mass2 = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
    body_set_centroid(mass2, (Vector) {30, 20});
    scene_add_body(scene, mass2);
    create_newtonian_gravity(scene, G, mass1, mass2);
    for (int i = 0; i < STEPS; i++) {
        assert(body_get_centroid(mass1).x < body_get_centroid(mass2).x);
        Vector v1 = body_get_velocity(mass1);
        Vector v2 = body_get_velocity(mass2);
        Vector momentum = vec_add(vec_multiply(M1, v1), vec_multiply(M2, v2));
        assert(vec_isclose(VEC_ZERO, momentum));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

void test_falling_object(){
    const double G = 7e-5;
    const double INIT_HEIGHT = 10;
    const double MASS = 5;
    const double EARTH_MASS = 6e24;
    const double EARTH_RADIUS = 7e9;
    const double INIT_VEL = 10;
    const double DT = 1e-6;
    const int STEPS = 100000;
    Scene *scene = scene_init();
    Body *mass = body_init(make_shape(), MASS, (RGBColor) {0, 0, 0});
    body_set_centroid(mass, (Vector) {0, INIT_HEIGHT});
    body_set_velocity(mass, (Vector) {0, INIT_VEL});
    scene_add_body(scene, mass);
    Body *anchor = body_init(make_shape(), EARTH_MASS, (RGBColor) {0, 0, 0});
    body_set_centroid(anchor, (Vector) {0, -EARTH_RADIUS});
    scene_add_body(scene, anchor);
    create_newtonian_gravity(scene, G, mass, anchor);
    double g_acc = G * EARTH_MASS / (EARTH_RADIUS * EARTH_RADIUS);
    for(int i = 0; i < STEPS; i++){
        double t = i * DT;
        double c_height = INIT_HEIGHT + INIT_VEL * t - 0.5 * g_acc * t * t;
        assert(vec_isclose(body_get_centroid(mass), (Vector) {0, c_height}));
        assert(isclose(body_get_velocity(mass).y, INIT_VEL - g_acc * t));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

void test_underdamped_harmonic_oscillator() {
    const double M = 10;
    const double K = 1.6;
    const double A = 3;
    const double DT = 1e-6;
    const double GAMMA = 0;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    /* test underdamped oscillator */
    body_set_centroid(mass, (Vector) {A, 0});
    scene_add_body(scene, mass);
    Body *anchor = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});
    scene_add_body(scene, anchor);
    create_spring(scene, K, mass, anchor);
    create_drag(scene, GAMMA, mass);
    for (int i = 0; i < STEPS; i++) {
        double w_d = sqrt(4 * M * K - GAMMA * GAMMA) / (2 * M);
        double n = -GAMMA * i * DT / (2 * M);
        assert(vec_isclose(
            body_get_centroid(mass),
            (Vector) {A * exp(n) * cos(w_d * i * DT), 0}
        ));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

void test_overdamped_harmonic_oscillator(){
    const double M = 10;
    const double K = 1.6;
    const double A = 3;
    const double DT = 1e-6;
    const double GAMMA = 10;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    /* test underdamped oscillator */
    body_set_centroid(mass, (Vector) {A, 0});
    scene_add_body(scene, mass);
    Body *anchor = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});
    scene_add_body(scene, anchor);
    create_spring(scene, K, mass, anchor);
    create_drag(scene, GAMMA, mass);
    for (int i = 0; i < STEPS; i++) {
        double r_1 = (-GAMMA + sqrt(GAMMA * GAMMA - 4 * M * K)) / (2 * M);
        double r_2 = (-GAMMA - sqrt(GAMMA * GAMMA - 4 * M * K)) / (2 * M);
        double c_1 = A / (1 - r_1 / r_2);
        double c_2 = A - c_1;
        assert(vec_isclose(
            body_get_centroid(mass),
            (Vector) {c_1 * exp(r_1 * i * DT) + c_2 * exp(r_2 * i * DT), 0}
        ));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}


/* group 03 */
Vector *vec_init_(double x, double y) {
    Vector *res = malloc(sizeof(Vector));
    res->x = x;
    res->y = y;
    return res;
}

void vec_free_(Vector *vec) {
    free(vec);
}

Body *n_polygon_shape_(size_t num_sides, double radius, double mass,
    RGBColor color, Vector centroid) {
    List *vertices = list_init(num_sides, (FreeFunc) vec_free_);
    double theta = 2 * M_PI / num_sides;
    Vector start = (Vector) {.x = radius, .y = 0};
    size_t i;

    for (i = 0; i < num_sides; i++) {
        Vector tmp = vec_rotate(start, theta * i);
        list_add(vertices, vec_init_(tmp.x, tmp.y));
    }

    Body *res = body_init(vertices, mass, color);
    body_set_centroid(res, centroid);
    return res;
}



double spring_potential_1(double K, Body *body1, Body *body2) {
    Vector r = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    return 0.5 * K * vec_dot(r, r);
}

// Tests that two same mass bodies have same kinetic energy
void test_same_kinetic() {
    const size_t num_sides = 4;
    const double radius = 1;
    const double M1 = 10, M2 = 10;
    const double G = 1e3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass1 = n_polygon_shape_(num_sides, radius, M1, (RGBColor) {0, 0, 0}, (Vector) {10, 100});
    scene_add_body(scene, mass1);
    Body *mass2 = n_polygon_shape_(num_sides, radius, M2, (RGBColor) {0, 0, 0}, (Vector) {1000, 100});
    scene_add_body(scene, mass2);
    create_newtonian_gravity(scene, G, mass1, mass2);
    for (int i = 0; i < STEPS; i++) {
        assert(within(1e-5, kinetic_energy(mass1), kinetic_energy(mass2)));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

// Tests if center body stays in same position for symmetric scene
void test_same_position() {
    const size_t num_sides = 4;
    const double radius = 1;
    const double M1 = 10, M2 = 1, M3 = 10;
    const double G = 1e3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass1 = n_polygon_shape_(num_sides, radius, M1, (RGBColor) {0, 0, 0}, (Vector) {0, 100});
    scene_add_body(scene, mass1);
    Body *mass2 = n_polygon_shape_(num_sides, radius, M2, (RGBColor) {0, 0, 0}, (Vector) {500, 100});
    scene_add_body(scene, mass2);
    Body *mass3 = n_polygon_shape_(num_sides, radius, M3, (RGBColor) {0, 0, 0}, (Vector) {1000, 100});
    scene_add_body(scene, mass3);
    create_newtonian_gravity(scene, G, mass1, mass2);
    create_newtonian_gravity(scene, G, mass2, mass3);
    create_newtonian_gravity(scene, G, mass1, mass3);
    for (int i = 0; i < STEPS; i++) {
        assert(vec_isclose(
            body_get_centroid(mass2),
            (Vector) {500, 100}
        ));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

// Tests conservation of spring and kinetic energy
void test_spring_energy() {
    const size_t num_sides = 4;
    const double radius = 1;
    const double M = 10;
    const double K = 2;
    const double A = 3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass = n_polygon_shape_(num_sides, radius, M, (RGBColor) {0, 0, 0}, (Vector) {A, 0});
    scene_add_body(scene, mass);
    Body *anchor = n_polygon_shape_(num_sides, radius, INFINITY, (RGBColor) {0, 0, 0}, (Vector) {0, 0});
    scene_add_body(scene, anchor);
    create_spring(scene, K, mass, anchor);
    double initial_energy = spring_potential_1(K, mass, anchor);
    for (int i = 0; i < STEPS; i++) {
        assert(within(1e-5, initial_energy, spring_potential_1(K, mass, anchor) + kinetic_energy(mass)));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}


/* group 04 */
const RGBColor COLOR_YELLOW_temp = {.r = 1, .g = 1, .b = 0};

Vector *vec_clone_temp(Vector v) {
  Vector *on_heap = malloc(sizeof(Vector));
  on_heap->x = v.x;
  on_heap->y = v.y;
  return on_heap;
}

double uniform_angle_spacing_temp(size_t num_vertices) {
  return 2 * M_PI / num_vertices;
}

List *polygon_init_regular_temp(size_t num_vertices, double radius) {
  List *polygon = list_init(num_vertices, free);

  double vertex_spacing_angle = uniform_angle_spacing_temp(num_vertices);
  Vector vector = {.x = radius, .y = 0};

  for (size_t i = 0; i < num_vertices; i++) {
    vector = vec_rotate(vector, vertex_spacing_angle);
    list_add(polygon, vec_clone_temp(vector));
  }

  return polygon;
}

// Test that no impulse or force does not change anything when tick
void simple_test(Body *test) {
  for (int i = 1; i < 10; i++) {
    body_tick(test, i);
    assert(body_get_velocity(test).x == VEC_ZERO.x);
    assert(body_get_centroid(test).x == VEC_ZERO.x);
    assert(body_get_velocity(test).y == VEC_ZERO.y);
    assert(body_get_centroid(test).y == VEC_ZERO.y);
  }
  body_free(test);
}

void opposing_forces(Body *test) {
  Vector force = {.x = 90, .y = 50};
  body_add_force(test, force);
  body_add_force(test, vec_negate(force));
  body_tick(test, 3.0);
  assert(body_get_velocity(test).x == VEC_ZERO.x);
  assert(body_get_centroid(test).x == VEC_ZERO.x);
  assert(body_get_velocity(test).y == VEC_ZERO.y);
  assert(body_get_centroid(test).y == VEC_ZERO.y);
  body_free(test);
}

void opposing_force_and_impulse(Body *test) {
  Vector force = {.x = 90, .y = 50};
  body_add_force(test, force);
  body_add_impulse(test, vec_negate(force));
  body_tick(test, 1.0);
  assert(body_get_velocity(test).x == VEC_ZERO.x);
  assert(body_get_centroid(test).x == VEC_ZERO.x);
  assert(body_get_velocity(test).y == VEC_ZERO.y);
  assert(body_get_centroid(test).y == VEC_ZERO.y);
  body_free(test);
}

void constant_acceleration(Body *test) {
  Vector force = {.x = 10, .y = 10};
  Vector velocity_base = {.x = 1, .y = 1};
  for (int i = 1; i < 11; i++) {
    body_add_force(test, force);
    body_tick(test, 1.0);
    assert(body_get_velocity(test).x == vec_multiply(i, velocity_base).x);
    assert(body_get_velocity(test).y == vec_multiply(i, velocity_base).y);
  }
  body_free(test);
}

// Test simple forces
void force_tests() {
  Body *body1 = body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  simple_test(body1);
  Body *body2 = body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body2, VEC_ZERO);
  opposing_forces(body2);
  Body *body3 = body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body3, VEC_ZERO);
  opposing_force_and_impulse(body3);
  Body *body4 = body_init(polygon_init_regular_temp(6, 30), 10, COLOR_YELLOW_temp);
  body_set_centroid(body4, VEC_ZERO);
  constant_acceleration(body4);
}

void grav_tests() {
  Scene *scene = scene_init();
  Vector trans = {.x = 10, .y = 0};
  Body *body1 =
      body_init(polygon_init_regular_temp(1, 30), 100, COLOR_YELLOW_temp);
  Body *body2 =
      body_init(polygon_init_regular_temp(1, 30), 100, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  body_set_centroid(body2, trans);
  scene_add_body(scene, body1);
  scene_add_body(scene, body2);
  create_newtonian_gravity(scene, 1, body1, body2);
  scene_tick(scene, 1);
  Vector velocity_1 = {.x = 1, .y = 0};
  printf("VEL\t%f\tY\t%f\n", body_get_velocity(body1).x, body_get_velocity(body1).y);
  assert(vec_isclose(body_get_velocity(body1), velocity_1));
  assert(vec_isclose(body_get_velocity(body2), vec_negate(velocity_1)));
  // assert(body_get_velocity(body1).x == velocity_1.x);
  // assert(body_get_velocity(body2).x == vec_negate(velocity_1).x);
  // assert(body_get_velocity(body1).y == velocity_1.y);
  // assert(body_get_velocity(body2).y == vec_negate(velocity_1).y);
  scene_free(scene);
}

void spring_tests() {
  Scene *scene = scene_init();
  Vector trans = {.x = 10, .y = 0};
  Body *body1 =
      body_init(polygon_init_regular_temp(6, 30), 100, COLOR_YELLOW_temp);
  Body *body2 =
      body_init(polygon_init_regular_temp(6, 30), 100, COLOR_YELLOW_temp);
  body_set_centroid(body1, VEC_ZERO);
  body_set_centroid(body2, trans);
  scene_add_body(scene, body1);
  scene_add_body(scene, body2);
  create_spring(scene, 10, body1, body2);
  scene_tick(scene, 1.0);
  Vector velocity_1 = {.x = 1, .y = 0};
  assert(body_get_velocity(body1).x == velocity_1.x);
  assert(body_get_velocity(body2).x == vec_negate(velocity_1).x);
  assert(body_get_velocity(body1).y == velocity_1.y);
  assert(body_get_velocity(body2).y == vec_negate(velocity_1).y);
  scene_free(scene);
}

void drag_tests() {
  Scene *scene = scene_init();
  Body *body =
      body_init(polygon_init_regular_temp(6, 30), 1, COLOR_YELLOW_temp);
  scene_add_body(scene, body);
  Vector velocity = {.x = 100, .y = 0};
  body_set_velocity(body, velocity);
  create_drag(scene, 10, body);
  for (int i = 0; i < 10; i++) {
    scene_tick(scene, 1.0);
    Vector del_v = vec_multiply(-10, velocity);
    velocity = vec_add(velocity, del_v);
    assert(body_get_velocity(body).x == velocity.x);
    assert(body_get_velocity(body).y == velocity.y);
  }
  scene_free(scene);
}


/* group 05 */
double spring_potential_2(double K, Body *body1, Body *body2) {
    Vector x = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    return 0.5 * K * vec_dot(x, x);
}

// Tests that a conservative force (spring) conserves K + U
void test_spring_energy_conservation_2() {
    const double M1 = 4.5, M2 = 7.3;
    const double K = 1;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass1 = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
    scene_add_body(scene, mass1);
    Body *mass2 = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
    body_set_centroid(mass2, (Vector) {10, 20});
    scene_add_body(scene, mass2);
    create_spring(scene, K, mass1, mass2);
    double initial_energy = spring_potential_2(K, mass1, mass2);
    for (int i = 0; i < STEPS; i++) {
        double energy = spring_potential_2(K, mass1, mass2) +
            kinetic_energy(mass1) + kinetic_energy(mass2);
        assert(within(1e-5, energy / initial_energy, 1));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

/* Test that applying drag force to a stationary object does not affect the
 * object.
 */
void test_stationary_drag() {
    const double M = 5.0;
    const double GAMMA = 100;
    const double DT = 1e-3;
    const int STEPS = 1000;
    Scene *scene = scene_init();
    Body *b = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    scene_add_body(scene, b);
    create_drag(scene, GAMMA, b);
    for (int i = 0; i < STEPS; i++) {
        assert(vec_equal(body_get_velocity(b), VEC_ZERO));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

/* Test that applying drag force to a moving object eventually stops the
 * object.
 */
void test_moving_drag() {
    const double M = 5.0;
    const double GAMMA = 100;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *b = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    body_set_velocity(b, (Vector){10, 20});
    scene_add_body(scene, b);
    create_drag(scene, GAMMA, b);
    for (int i = 0; i < STEPS; i++) {
        scene_tick(scene, DT);
    }
    assert(vec_isclose(body_get_velocity(b), VEC_ZERO));
    scene_free(scene);
}


/* group 06 */
/*void test_drag() {
  const double M = 10;
  const double gamma = .9;
  const double Vi = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  double elapsed = 0;
  Scene *scene = scene_init();
  Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_velocity(mass, (Vector) {Vi, 0});
  scene_add_body(scene, mass);
  create_drag(scene, gamma, mass);
  for (int i = 0; i < STEPS; i++) {
      elapsed = i * DT;
      // printf("\nVelocity Real: {%f, %f}\n", body_get_velocity(mass).x, body_get_velocity(mass).y);
      // printf("\nVelocity Expected: {%f, %f}\n", ((Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}).x, ((Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}).y);

      assert(vec_isclose(
          body_get_velocity(mass),
          (Vector) {Vi * pow(M_E, -gamma * elapsed / M), 0}
      ));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_orbit() {
  const double M1 = 1.0, M2 = 1.0 * 1e5;
  const double G = 10;
  const double DT = 1e-5;
  const int STEPS = 500000;
  Scene *scene = scene_init();
  Body *earth = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
  scene_add_body(scene, earth);
  body_set_centroid(earth, (Vector) {100, 0});
  body_set_velocity(earth, (Vector) {0, 100});
  Body *sun = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
  scene_add_body(scene, sun);
  create_newtonian_gravity(scene, G, earth, sun);
  for (int i = 0; i < STEPS; i++) {
      // printf("\nDistance Vec: %f\n", vec_distance(body_get_centroid(earth), body_get_centroid(sun)));
      // printf("Distance body: %f\n", body_distance(earth, sun));

      assert(within(1e-2, vec_distance(body_get_centroid(earth), body_get_centroid(sun)), 100));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

double spring_energy(Body *b1, Body *b2, double k){
  return .5 * k * pow(vec_distance(body_get_centroid(b1), body_get_centroid(b2)), 2);
}

void test_spring_energy_2(){
  const double M = 10;
  const double K = 2;
  const double A = 3;
  const double DT = 1e-6;
  const int STEPS = 1000000;
  Scene *scene = scene_init();
  Body *mass1 = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass1, (Vector) {A, 0});
  scene_add_body(scene, mass1);
  Body *mass2 = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
  body_set_centroid(mass2, (Vector) {-A, 0});
  scene_add_body(scene, mass2);
  create_spring(scene, K, mass1, mass2);
  double initial_energy = spring_energy(mass1, mass2, K);

  for (int i = 0; i < STEPS; i++) {
    double energy = spring_energy(mass1, mass2, K) +
        kinetic_energy(mass1) + kinetic_energy(mass2);
      assert(within(1e-2, energy, initial_energy));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}*/

/* group 07 */



double spring_potential_3(Body *body1, Body *body2, double constant) {
    Vector x = vec_subtract(body_get_centroid(body1), body_get_centroid(body2));
    return constant * vec_dot(x, x) / 2;
}

double spring_total(Body *anchor, Body *mass, double constant) {
    return kinetic_energy(mass) + spring_potential_3(anchor, mass, constant);
}

double body_total(double G, Body *body1, Body *body2){
    return gravity_potential(G, body1, body2) + kinetic_energy(body1) + kinetic_energy(body2);
}



// Tests that the total energy of a spring system decreases over time (damping).
void test_spring_damping() {
    const double M = 10;
    const double A = 10;
    const double K = 10;
    const double DT = 1e-1;
    const int STEPS = 100;

    Scene *scene = scene_init();

    Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    body_set_centroid(mass, (Vector) {A, 0});
    scene_add_body(scene, mass);

    Body *anchor = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});
    body_set_centroid(anchor, (Vector) {0, 0});
    scene_add_body(scene, anchor);

    double peak_energy1, peak_energy2, energy1, energy2, energy3;
    peak_energy1 = 0;
    peak_energy2 = 0;
    energy1 = spring_total(anchor, mass, K);
    scene_tick(scene, DT);
    energy2 = spring_total(anchor, mass, K);
    scene_tick(scene, DT);
    energy3 = spring_total(anchor, mass, K);

    for (int i = 0; i < STEPS; i++) {
        scene_tick(scene, DT);
        energy1 = energy2;
        energy2 = energy3;
        energy3 = spring_total(anchor, mass, K);

        if (energy2 > energy3 && energy2 > energy1) {
            if (peak_energy2 == 0) {
                peak_energy2 = energy2;
            }
            else {
                peak_energy1 = peak_energy2;
                peak_energy2 = energy2;

                assert(peak_energy2 < peak_energy1);
            }
        }

    }
    scene_free(scene);
}

// Tests that the total energy of a system decreases over time (drag).
void test_drag_2() {
    const double M1 = 4.5, M2 = 7.3;
    const double G = 1e3;
    const double DT = 1e-3;
    const int STEPS = 1000;
    Scene *scene = scene_init();
    Body *mass1 = body_init(make_shape(), M1, (RGBColor) {0, 0, 0});
    scene_add_body(scene, mass1);
    Body *mass2 = body_init(make_shape(), M2, (RGBColor) {0, 0, 0});
    body_set_centroid(mass2, (Vector) {10, 20});
    scene_add_body(scene, mass2);
    create_newtonian_gravity(scene, G, mass1, mass2);
    create_drag(scene, 2, mass2);
    create_drag(scene, 2, mass1);
    double initial_energy = body_total(G, mass1, mass2);

    for (int i = 0; i < STEPS; i++) {
        scene_tick(scene, DT);
        double energy = body_total(G, mass1, mass2);
        assert(energy < initial_energy);
        initial_energy = energy;
    }
    scene_free(scene);
}

void test_freefall() {
    const double M = 5;
    const double G = 1e3;
    const double DT = 1e-3;
    const int STEPS = 1000;
    Scene *scene = scene_init();
    Body *mass_drag = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    Body *mass_no_drag = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    Body *earth = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});

    scene_add_body(scene, mass_drag);
    scene_add_body(scene, mass_no_drag);
    scene_add_body(scene, earth);

    body_set_centroid(earth, (Vector) {-1000, -1000});

    create_newtonian_gravity(scene, G, mass_drag, earth);
    create_newtonian_gravity(scene, G, mass_no_drag, earth);\

    create_drag(scene, 2, mass_drag);
    for (int i = 0; i < STEPS; i++) {
        scene_tick(scene, DT);

        if (within(pow(1e-5, 2), vec_dot(vec_subtract(body_get_centroid(mass_no_drag), body_get_centroid(earth)),
                                         vec_subtract(body_get_centroid(mass_no_drag), body_get_centroid(earth))), 0))
        {
            assert(!within(pow(1e-5, 2), vec_dot(vec_subtract(body_get_centroid(mass_drag), body_get_centroid(earth)),
                                                 vec_subtract(body_get_centroid(mass_drag), body_get_centroid(earth))), 0));
            break;
        }
    }
    scene_free(scene);
}


/* group 09
const double G = 6.67408e-11;
const double DT = 1e-12;
const int STEPS = 100;

double distance_vec(Vector v1, Vector v2){
  Vector vec_diff = vec_subtract(v1, v2);
  return sqrt(vec_dot(vec_diff, vec_diff));
}

double get_distance_between_centroids(Body *body1, Body *body2){
  return distance_vec(body_get_centroid(body1), body_get_centroid(body2));
}


// Gets new velocity of body1 due to body2 after time DT
Vector get_new_velocity_grav(Body *body1, Body *body2, Vector prev_vel_1){
  double acceleration_1 = G * body_get_mass(body2) / pow(
      get_distance_between_centroids(body1, body2), 3);
  Vector direction_1_to_2 = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
  Vector delta_v_1 = vec_multiply(acceleration_1 * DT, direction_1_to_2);
  return vec_add(delta_v_1, prev_vel_1);
}

void assert_grav(Scene *scene, Body *body1, Body *body2){
  for (int i = 0; i < STEPS; i++) {
      Vector prev_vel_1 = body_get_velocity(body1);
      Vector prev_vel_2 = body_get_velocity(body2);
      create_newtonian_gravity(scene, G, body1, body2);

      assert(vec_within(1e-3, body_get_velocity(body1), get_new_velocity_grav(body1, body2, prev_vel_1)));
      assert(vec_within(1e-3, body_get_velocity(body2), get_new_velocity_grav(body2, body1, prev_vel_2)));

      scene_tick(scene, DT);
  }
  scene_free(scene);
}

Scene *scene_maker(Vector distance, double m1, double m2){
  Scene *scene = scene_init();
  Body *mass1 = body_init(make_shape(), m1, (RGBColor) {0, 0, 0});
  scene_add_body(scene, mass1);
  Body *mass2 = body_init(make_shape(), m2, (RGBColor) {0, 0, 0});
  body_set_centroid(mass2, distance);
  scene_add_body(scene, mass2);
  return scene;
}

// Tests the gravity between different pairs of bodies. Bodies that are close
// should not have a force
void test_newtonian_gravity(){

  //Case where two bodies of the same size are relatively close
  Scene *scene = scene_maker((Vector){30, 50}, 3, 3);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where there are different bodies that are relatively close
  scene = scene_maker((Vector){30, 50}, 3, 40);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where the different bodies are far apart
  scene = scene_maker((Vector){400, 340}, 3, 40);
  assert_grav(scene, scene_get_body(scene, 0), scene_get_body(scene, 1));

  //Case where they are really close
  scene = scene_maker((Vector){3, 5}, 3, 40);
  Body *body0 = scene_get_body(scene, 0);
  Body *body1 = scene_get_body(scene, 1);
  Vector prev_vel_1 = body_get_velocity(body0);
  Vector prev_vel_2 = body_get_velocity(body1);
  create_newtonian_gravity(scene, G, body0, body1);
  assert(vec_within(1e-3, prev_vel_1, body_get_velocity(body0)));
  assert(vec_within(1e-3, prev_vel_2, body_get_velocity(body1)));
  scene_free(scene);
}

// Gets new velocity of mass after time DT
Vector get_new_velocity_spring(Body *mass, Body *anchor, double k, Vector prev_vel){
  double scalar = -k / body_get_mass(mass);
  Vector distance = vec_subtract(body_get_centroid(anchor), body_get_centroid(mass));
  Vector acceleration = vec_multiply(scalar, distance);
  return vec_add(vec_multiply(DT, acceleration), prev_vel);
}

typedef struct {
  Scene *scene;
  double k;
} Spring;


void create_spring_wrapper(Spring *spring){
  create_spring(spring->scene, spring->k, scene_get_body(spring->scene, 0),
      scene_get_body(spring->scene, 1));
}

Spring *spring_init_1(Scene *scene, double k){
  Spring *spring = malloc(sizeof(Spring));
  spring->scene = scene;
  spring->k = k;
  return spring;
}

void spring_free(Spring *spring){
  scene_free(spring->scene);
  free(spring);
}

// Mass will oscillate according to anchor. This checks the velocity of the mass.
// Anchor is at equilibrium point.
void assert_spring(Scene *scene, double k, Body *mass, Body *anchor){
  for (int i = 0; i < STEPS; i++) {
      Vector prev_vel = body_get_velocity(mass);
      create_spring(scene, k, mass, anchor);
      assert(vec_within(1e-3, body_get_velocity(mass), get_new_velocity_spring(mass, anchor, k, prev_vel)));
      scene_tick(scene, DT);
  }
  scene_free(scene);
}

void test_spring(){
  // Case 1: Assert fails because negative k.
  double k0 = -234823905234682905;
  Scene *scene = scene_maker((Vector){1, 1}, 3, 3);
  Spring *spring = spring_init_1(scene, k0);
  test_assert_fail((ForceCreator) create_spring_wrapper, (void *)spring);
  spring_free(spring);

  // Case 2: Assert fails because zero k.
  double k1 = 0;
  scene = scene_maker((Vector){1, 1}, 3, 3);
  spring = spring_init_1(scene, k1);
  test_assert_fail((ForceCreator) create_spring_wrapper, (void *)spring);
  spring_free(spring);

  // Case 3: Very weak spring.
  double k2 = 1;
  scene = scene_maker((Vector){50, 100}, 10, 20);
  assert_spring(scene, k2, scene_get_body(scene, 0), scene_get_body(scene, 1));

  // Case 4: Strong spring, close distance.
  double k3 = 50;
  scene = scene_maker((Vector){25, 50}, 10, 20);
  assert_spring(scene, k3, scene_get_body(scene, 0), scene_get_body(scene, 1));

  // Use k3 with 2 different distances
  scene = scene_maker((Vector){300, 500}, 3, 3);
  assert_spring(scene, k3, scene_get_body(scene, 0), scene_get_body(scene, 1));
}

void assert_drag(Scene *scene, double gamma, Vector velocity){
  Body *body = scene_get_body(scene, 0);
  body_set_velocity(body, velocity);

  for (int i = 0; i < STEPS; i++) {
    Vector prev_vel = body_get_velocity(body);
    create_drag(scene, gamma, body);
    Vector force = vec_multiply(-1 * gamma, prev_vel);
    Vector new_vel = vec_multiply(DT / body_get_mass(body), force);
    new_vel = vec_add(new_vel, prev_vel);
    scene_tick(scene, DT);
    assert(vec_within(1e-3, body_get_velocity(body), new_vel));
  }
}

void test_drag_3(){
  Scene *scene = scene_maker((Vector){50, 100}, 3, 3);
  Vector velocity = {0, 400};
  double gamma = 10;

  // Case 1: Velocity 0, no drag.
  assert_drag(scene, gamma, VEC_ZERO);

  // Case 2: North velocity
  assert_drag(scene, gamma, velocity);

  // Case 3: Northeast velocity
  assert_drag(scene, gamma / 10, vec_rotate(velocity, - M_PI / 4));

  // Case 4: Southeast velocity
  assert_drag(scene, gamma * 3, vec_rotate(velocity, - M_PI / 2));

  // Case 5: Southwest velocity
  assert_drag(scene, gamma * 10, vec_rotate(velocity, - M_PI / 2));

  scene_free(scene);
}
*/

int main(int argc, char *argv[]) {
  // Run all tests if there are no command-line arguments
  bool all_tests = argc == 1;
  // Read test name from file
  char testname[100];
  if (!all_tests) {
      read_testname(argv[1], testname, sizeof(testname));
  }

  DO_TEST(test_period);
  DO_TEST(test_newton_third_law);
  DO_TEST(test_spring_energy_conservation);

  DO_TEST(test_momentum_conservation);
  DO_TEST(test_falling_object);
  DO_TEST(test_underdamped_harmonic_oscillator);
  DO_TEST(test_overdamped_harmonic_oscillator);

  DO_TEST(test_same_kinetic);
  DO_TEST(test_same_position);
  DO_TEST(test_spring_energy);

  // DO_TEST(force_tests);
  // DO_TEST(grav_tests);
  // DO_TEST(spring_tests);
  // DO_TEST(drag_tests);

  DO_TEST(test_spring_energy_conservation_2);
  DO_TEST(test_stationary_drag);
  DO_TEST(test_moving_drag);


  // DO_TEST(test_orbit);
  // DO_TEST(test_drag);
  // DO_TEST(test_spring_energy_2);

  DO_TEST(test_spring_damping);
  DO_TEST(test_drag_2);
  DO_TEST(test_freefall);


  // DO_TEST(test_newtonian_gravity);
  // DO_TEST(test_spring);
  // DO_TEST(test_drag_3);

  puts("student_test PASS");
  return 0;
}
