#include "tileset.h"
#include <dirent.h>
#include <stdio.h>

typedef struct { 
    int tile_id;
    int tile_health;
    int other_data;
    CollisionType collision;
    SDL_Texture *block_texture;
    Mix_Chunk *damage_sound;
} TileData;

void tiledata_free(void *t) {
    SDL_Texture *texture = ((TileData *) t)->block_texture;
    SDL_DestroyTexture(texture);
    free(t);
}

List *tileset_init(char *path_to_data) {
    int block_count;
    FILE *f = fopen(path_to_data, "r");
    fscanf(f, "%d", &block_count);
    List *temp = list_init(block_count, tiledata_free);

    for (int i = 0; i < block_count; i++) {
        TileData *holder = malloc(sizeof(TileData));
        char *path = malloc(sizeof(char) * 100);
        char *full_path = malloc(sizeof(char) * 100);
        int other, collision;
        fscanf(f, "%s %d %d %d %d", path, &holder->tile_id, &holder->tile_health, &other, &collision);
        holder->other_data = other, holder->collision = (CollisionType) collision;
        sprintf(full_path, "resources/textures/blocks/%s", path);
        // printf("HEALTH: %d\tID: %d\tPATH: %s \n", holder->tile_health,holder->tile_id,full_path);
        holder->block_texture = sdl_load_image(full_path);
        list_add(temp, (void *) holder);
        free(path);
        free(full_path);
    }
    fclose(f);
    return temp;
}

SDL_Texture *tileset_get_texture(List *tileset, size_t i) {
    return ((TileData*) list_get(tileset, i))->block_texture;
}

CollisionType tileset_get_collision(List *tileset, size_t i) {
    return ((TileData *) list_get(tileset, i))->collision;
}

int tileset_get_health(List *tileset, size_t i) {
    return ((TileData*) list_get(tileset, i))->tile_health;
}

int tileset_get_id(List *tileset, size_t i) {
    return ((TileData*)list_get (tileset, i))->tile_id;
}

void tileset_play_damage_sound(List *tileset, size_t i) {
    play_audio_chunk(((TileData*) list_get(tileset, i))->damage_sound, 1);
}

bool tileset_is_flag(void *tile) { 
    return ((TileData *) tile)->other_data == 1;
}

bool tileset_index_is_flag(List *tileset, size_t i) { 
    return ((TileData*) list_get(tileset, i))->other_data == 1;
}

bool tileset_is_cannon(void *tile) {
    return ((TileData *) tile)->other_data == 2;
}