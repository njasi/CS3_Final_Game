#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include "network_engine.h"

#define VECTOR(x, y) ((Vector) {(x), (y)})

#define OVER1(over) ((*(over)) = 1)
#define OVER2(over) ((*(over)) = 2)
#define SWITCHTURN(turn) ((*(turn)) = !(*(turn)))
#define CHANGE(idx) ((*(idx)) = ++(*(idx)) % 5)
#define ADDOBJECT(scene, body_id, type) (scene_create_body_type((scene), (body_id), (type)))
#define REMOVEOBJECT(scene, body_id) (scene_remove_body_id((scene), (body_id)))
#define ADDFORCE(scene, x, y, body_id) (body_add_force(scene_get_body_id((scene), (body_id)), VECTOR((x), (y))))
#define ADDIMPULSE(scene, x, y, body_id) (body_add_impulse(scene_get_body_id((scene), (body_id)), VECTOR((x), (y))))
#define SETVELOCITY(scene, v, body_id) (body_set_velocity(scene_get_body_id((scene), (body_id)), v))
#define ROTATE(scene, body_id, angle) (body_set_rotation(scene_get_body_id((scene), (body_id)), (angle)))

#define PARSE(t, d, n) \
    for (int i = 0; i < (n); i++) { \
        t = strtok(NULL, d); \
    }

void parse_command(Scene *scene, char command[], void *data) {
    const char *delim = " [,]\0";
    char *token = strtok(command, delim);

    if (strcmp(token, "OVER1") == 0) {
        int *over = (int *) data;
        OVER1(over);
    }
    else if (strcmp(token, "OVER2") == 0) {
        int *over = (int *) data;
        OVER2(over);
    }
    else if (strcmp(token, "SWITCHTURN") == 0) {
        bool *turn = (bool *) data;
        SWITCHTURN(turn);
    }
    else if (strcmp(token, "CHANGE") == 0) {
        int *idx = (int *) data;
        CHANGE(idx);
    }
    else if (strcmp(token, "ADDOBJECT") == 0) {
        int *counter = (int *) data;
        (*counter)++;
        PARSE(token, delim, 1);
        int body_id = atoi(token);
        PARSE(token, delim, 1);
        int type = atoi(token);
        PARSE(token, delim, 2);
        double x = atof(token);
        PARSE(token, delim, 1);
        double y = atof(token);
        ADDOBJECT(scene, body_id, type);
        Vector *vec = malloc(sizeof(Vector));
        vec->x = x, vec->y = y;
        SETVELOCITY(scene, *vec, *counter);
        free(vec);
    }
    else if (strcmp(token, "REMOVEOBJECT") == 0) {
        PARSE(token, delim, 1);
        REMOVEOBJECT(scene, atoi(token));
    }
    else if (strcmp(token, "ADDFORCE") == 0) {
        PARSE(token, delim, 2);
        double x = atof(token);
        PARSE(token, delim, 1);
        double y = atof(token);
        PARSE(token, delim, 2);
        int body_id = atoi(token);
        ADDFORCE(scene, x, y, body_id);
    }
    else if (strcmp(token, "ADDIMPULSE") == 0) {
        PARSE(token, delim, 2);
        double x = atof(token);
        PARSE(token, delim, 1);
        double y = atof(token);
        PARSE(token, delim, 2);
        int body_id = atoi(token);
        ADDIMPULSE(scene, x, y, body_id);
    }
    else if (strcmp(token, "ROTATE") == 0) {
        PARSE(token, delim, 1);
        int body_id = atoi(token);
        PARSE(token, delim, 1);
        double angle = atof(token);
        ROTATE(scene, body_id, angle);
    }
}

