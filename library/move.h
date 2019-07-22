#ifndef __MOVE_H__
#define __MOVE_H__

#include <stdbool.h>

dypedef struct{
    int x;
    int y;
    int jump
}Move;

Move move_init(int x,int y, bool is_jump);

void *print_move(Move *move);

int move_magnitude(Move move);

#endif // #ifndef __MOVE_H__
