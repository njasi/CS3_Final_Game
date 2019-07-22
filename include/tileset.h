#ifndef __TILESET_H__
#define __TILESET_H__

#include <stddef.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <assert.h>
#include "list.h"
#include "vector.h"
#include "sdl_wrapper.h"

#define CANNON_VALUE 279
#define TARGET_VALUE 453

typedef enum {
    GHOST,
    INVINCIBLE,
    DESTRUCTIVE,
    DAMAGE
} CollisionType;

void tileset_free(void *t);

/**
 * will load in a tileset given it's directory
 *
 * @param path_to_data the path to the tileset dir
 */
List *tileset_init(char *path_to_data);

/**
 * get the tile texture at the index
 * @param tileset the tileset you want to get the texture from
 * @param i the index of the desired tile
 */
SDL_Texture *tileset_get_texture(List *tileset, size_t i);


/**
 * get what type of collision the tile should have
 *
 * @param tileset the tileset you want to get the collision boolean from
 * @param i the index of the desired tile
 */
CollisionType tileset_get_collision(List *tileset, size_t i);

/**
 * get the tile health at the index
 *
 * @param tileset the tileset you want to get the health from
 * @param i the index of the desired tile
 */
int tileset_get_health(List *tileset, size_t i);

/**
 * get the tile id at the index. This function is useless unless the tiles are made wrong
 *
 * @param tileset the tileset you want to get the id from
 * @param i the index of the desired tile (same as id)
 */
int tileset_get_id(List *tileset, size_t i);

/**
 * plays the sound associated with that tile taking damage
 *
 * @param tileset the tileset that you want the sound from
 * @param i the index of the desired tile (same as id)
 */
void tileset_play_damage_sound(List *tileset, size_t i);

/**
 * determines if the given tile is a flag
 *
 * @param tile the TileData struct you want to check
 */
bool tileset_is_flag(void *tile);


/**
 * determines if the given tile is a flag
 *
 * @param index
 */
bool tileset_index_is_flag(List *tileset, size_t i);

/**
 * determines if the given tile is a cannon
 *
 * @param tile the TileData struct you want to check
 */
bool tileset_is_cannon(void *tile);

#endif // #ifndef __TILESET_H__
