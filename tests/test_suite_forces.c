#include "forces.h"
#include "test_util.h"
#include <assert.h>
#include <math.h>
#include <stdlib.h>

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

// Tests that a mass on a spring oscillates like A cos(sqrt(K / M) * t)
void test_spring_sinusoid() {
    const double M = 10;
    const double K = 2;
    const double A = 3;
    const double DT = 1e-6;
    const int STEPS = 1000000;
    Scene *scene = scene_init();
    Body *mass = body_init(make_shape(), M, (RGBColor) {0, 0, 0});
    body_set_centroid(mass, (Vector) {A, 0});
    scene_add_body(scene, mass);
    Body *anchor = body_init(make_shape(), INFINITY, (RGBColor) {0, 0, 0});
    scene_add_body(scene, anchor);
    create_spring(scene, K, mass, anchor);
    for (int i = 0; i < STEPS; i++) {
      assert(vec_isclose(
          body_get_centroid(mass),
          (Vector) {A * cos(sqrt(K / M) * i * DT), 0}
      ));
      scene_tick(scene, DT);
    }
    scene_free(scene);
}

double gravity_potential(double G, Body *body1, Body *body2) {
    Vector r = vec_subtract(body_get_centroid(body2), body_get_centroid(body1));
    return -G * body_get_mass(body1) * body_get_mass(body2) / sqrt(vec_dot(r, r));
}
double kinetic_energy(Body *body) {
    Vector v = body_get_velocity(body);
    return body_get_mass(body) * vec_dot(v, v) / 2;
}

// Tests that a conservative force (gravity) conserves K + U
void test_energy_conservation() {
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
        assert(body_get_centroid(mass1).x < body_get_centroid(mass2).x);
        double energy = gravity_potential(G, mass1, mass2) +
            kinetic_energy(mass1) + kinetic_energy(mass2);
        assert(within(1e-5, energy / initial_energy, 1));
        scene_tick(scene, DT);
    }
    scene_free(scene);
}

int main(int argc, char *argv[]) {
    // Run all tests if there are no command-line arguments
    bool all_tests = argc == 1;
    // Read test name from file
    char testname[100];
    if (!all_tests) {
        read_testname(argv[1], testname, sizeof(testname));
    }

    DO_TEST(test_spring_sinusoid);
    DO_TEST(test_energy_conservation);

    puts("forces_test PASS");
    return 0;
}
