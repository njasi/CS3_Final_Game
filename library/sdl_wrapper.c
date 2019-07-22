#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <time.h>
#include "sdl_wrapper.h"

#define WINDOW_TITLE "Castle Breakers CDXX LXIX"
#define WINDOW_WIDTH 1000
#define WINDOW_HEIGHT 500
#define MS_PER_S 1e3

/**
 * The coordinate at the center of the screen.
 */
Vector center;

/**
 * The coordinate difference from the center to the top right corner.
 */
Vector max_diff;

/**
 * Current Scrolling Speed
 */
Vector scroll_vel;

/**
 * Current Scrolling position
 */
Vector scroll_pos;

/**
 * The SDL window where the scene is rendered.
 */
SDL_Window *window;

/**
 * The renderer used to draw the scene.
 */
SDL_Renderer *renderer;

/**
 * The font used to render text in this window.
 */
TTF_Font *font = NULL;

/**
 * The keypress handler, or NULL if none has been configured.
 */
KeyHandler key_handler = NULL;

/**
 * the aux for the key handler
 */
void *key_data;

double zoom_amount;
double zoom_rate;
Vector zoom_range;

/**
 * the mouse movement/click/and relase handler
 */
MouseHandler mouse_handler = NULL;

/**
 * the aux for the mouse handler
 */
void *mouse_data;

/**
 * SDL's timestamp when a key was last pressed or released.
 * Used to mesasure how long a key has been held.
 */
uint32_t key_start_timestamp;
/**
 * The value of clock() when time_since_last_tick() was last called.
 * Initially 0.
 */
clock_t last_clock = 0;

/**
 * Converts an SDL key code to a char.
 * 7-bit ASCII characters are just returned
 * and arrow keys are given special character codes.
 */
char get_keycode(SDL_Keycode key) {
    switch (key) {
        case SDLK_LEFT: return LEFT_ARROW;
        case SDLK_UP: return UP_ARROW;
        case SDLK_RIGHT: return RIGHT_ARROW;
        case SDLK_DOWN: return DOWN_ARROW;
        case SDLK_SPACE: return SPACEBAR;
        case SDLK_EQUALS: return EQUALS;
        case SDLK_MINUS: return MINUS;
        case SDLK_TAB: return TAB;
        default:
            // Only process 7-bit ASCII characters
            return key == (SDL_Keycode) (char) key ? key : '\0';
    }
}

void sdl_init(Vector min, Vector max) {
    // Check parameters
    assert(min.x < max.x);
    assert(min.y < max.y);
    zoom_range = (Vector){1,1};
    zoom_rate = 0;
    zoom_amount = 1;
    center = vec_multiply(0.5, vec_add(min, max));
    max_diff = vec_subtract(max, center);
    SDL_Init(SDL_INIT_EVERYTHING); // start sdl
    window = SDL_CreateWindow(
        WINDOW_TITLE,
        SDL_WINDOWPOS_CENTERED,
        SDL_WINDOWPOS_CENTERED,
        max.x-min.x,
        max.y-min.y,
        SDL_WINDOW_RESIZABLE
    );
    renderer = SDL_CreateRenderer(window, -1, 0);
    TTF_Init(); 
    int flags=IMG_INIT_JPG|IMG_INIT_PNG;
    int initted=IMG_Init(flags);
    if((initted&flags) != flags) {
        printf("IMG_Init: Failed to init required jpg and png support!\n");
        printf("IMG_Init: %s\n", IMG_GetError());
    }
    Mix_OpenAudio( 44100, MIX_DEFAULT_FORMAT, 2, 2048 ); // _AUDIO_
}

void sdl_set_font(char *font_path, size_t size){
    font = TTF_OpenFont(font_path, size);
    if(font == NULL){
        printf("Could not load the font: %s", font_path);
        exit(1);
    }
}

void mouse_event(int x, int y, MouseEventType type){
    if(!mouse_handler) return;
    mouse_handler(x, y, type, mouse_data);
}

bool sdl_is_done(void) {
    SDL_Event *event = malloc(sizeof(*event));
    assert(event);
    while (SDL_PollEvent(event)) {
        switch (event->type) {
            case SDL_QUIT:
                free(event);
                return true;
                TTF_Quit();
            case SDL_KEYDOWN:
            case SDL_KEYUP:
                // Skip the keypress if no handler is configured
                // or an unrecognized key was pressed
                if (!key_handler) break;
                char key = get_keycode(event->key.keysym.sym);
                if (!key) break;

                double timestamp = event->key.timestamp;
                if (!event->key.repeat) {
                    key_start_timestamp = timestamp;
                }
                KeyEventType type =
                    event->type == SDL_KEYDOWN ? KEY_PRESSED : KEY_RELEASED;
                double held_time =
                    (timestamp - key_start_timestamp) / MS_PER_S;
                key_handler(key, type, held_time, key_data);
                break;
            case SDL_MOUSEMOTION:
                // mouse moved
                mouse_event(event->motion.x, event->motion.y, MOUSEMOTION);
                break;
            case SDL_MOUSEBUTTONDOWN:
                // mouse clicked
                mouse_event(event->button.x, event->button.y, MOUSEBUTTONDOWN);
                break;
            case SDL_MOUSEBUTTONUP:
                // mouse released
                mouse_event(event->button.x, event->button.y, MOUSEBUTTONUP);
                break;
        }
    }
    free(event);
    return false;
}

void sdl_clear(void) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    SDL_RenderClear(renderer);
}

Vector get_center(void){
    int *width = malloc(sizeof(*width)),
        *height = malloc(sizeof(*height));
    assert(width);
    assert(height);
    SDL_GetWindowSize(window, width, height);
    double center_x = *width / 2.0,
           center_y = *height / 2.0;
    free(width);
    free(height);
    return vec_multiply(zoom_amount,(Vector){center_x,center_y});
}

double get_scale(Vector cen){
    double x_scale = cen.x / max_diff.x,
           y_scale = cen.y / max_diff.y;
    double scale = x_scale < y_scale ? x_scale : y_scale;
    return scale * zoom_amount;
}

void sdl_draw_polygon(List *points, RGBColor color) {
    // Check parameters
    size_t n = list_size(points);
    assert(n >= 3);
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    // Scale scene so it fits entirely in the window,
    // with the center of the scene at the center of the window

    Vector cen = get_center();
    double scale = get_scale(cen);

    // Convert each vertex to a point on screen
    short *x_points = malloc(sizeof(*x_points) * n),
          *y_points = malloc(sizeof(*y_points) * n);
    assert(x_points);
    assert(y_points);
    for (size_t i = 0; i < n; i++) {
        Vector *vertex = list_get(points, i);
        Vector pos_from_center =
            vec_multiply(scale, vec_subtract(*vertex, center));
        // Flip y axis since positive y is down on the screen
        x_points[i] = round(cen.x + pos_from_center.x + scroll_pos.x);
        y_points[i] = round(cen.y - pos_from_center.y + scroll_pos.y);
    }

    // Draw polygon with the given color
    filledPolygonRGBA(
        renderer,
        x_points, y_points, n,
        color.r * 255, color.g * 255, color.b * 255, 255
    );
    free(x_points);
    free(y_points);
}

void sdl_render_text(Vector pos, char *text, RGBColor color) {
    if(font == NULL){
        printf("Font was not initilaized before drawing, please use sdl_set_font().\n");
        exit(1);
    }
    assert(0 <= color.r && color.r <= 1);
    assert(0 <= color.g && color.g <= 1);
    assert(0 <= color.b && color.b <= 1);

    Vector cen = get_center();
    double scale = get_scale(cen);
    Vector pos_from_center = vec_multiply(scale, vec_subtract(pos, center));

    SDL_Color textColor = {color.r * 255, color.g * 255, color.b * 255, 0};

    SDL_Surface *surface = TTF_RenderText_Solid(font, text, textColor);
    SDL_Texture *texture = SDL_CreateTextureFromSurface(renderer, surface);
    if (texture == NULL){
        fprintf(stderr, "CreateTextureFromSurface failed: %s\nString: %s\n", SDL_GetError(), text);
        exit(1);
    }
    SDL_Rect *rect = malloc(sizeof(SDL_Rect));
    rect->w = surface->w;
    rect->h = surface->h;
    rect->x = round(cen.x + pos_from_center.x + scroll_pos.x);
    rect->y = round(cen.y - pos_from_center.y + scroll_pos.y);

    SDL_FreeSurface(surface);
    SDL_RenderCopy(renderer, texture, NULL, rect);
    free(rect);
}

void sdl_render_fixed_text(Vector pos, char *text, RGBColor color){
    sdl_render_text(vec_multiply(1 / zoom_amount, vec_subtract(pos, vec_multiply(1 / zoom_amount, scroll_pos))), text, color);
}

SDL_Texture *sdl_load_image(char *image_path){
    SDL_Surface *temp = IMG_Load(image_path);
    if(temp == NULL){
        printf("Failed to load %s\n",image_path);
        exit(1);
    }
    SDL_Texture *tex = SDL_CreateTextureFromSurface(renderer, temp);
    if (tex == NULL) {
        fprintf(stderr, "CreateTextureFromSurface failed: %s\nFile: %s\n", SDL_GetError(), image_path);
        exit(1);
    }
    SDL_FreeSurface(temp);
    return tex;
}

void sdl_render_image(Vector pos, Vector size, SDL_Texture *image, bool centered, double scroll_speed) {
    Vector cen = get_center();
    double scale = get_scale(cen);
    Vector pos_from_center = vec_multiply(scale, vec_subtract(pos, center));

    SDL_Rect placement = { cen.x + pos_from_center.x + scroll_speed * scroll_pos.x,
                           cen.y - pos_from_center.y + scroll_speed * scroll_pos.y,
                           size.x, size.y };
    if(centered) {
        placement.x -= size.x / 2;
        placement.y += size.y / 2;
    }
    SDL_RenderCopy(renderer, image, NULL, &placement);
}

void sdl_render_rotated_image(Vector pos, Vector size, SDL_Texture *image, bool centered, Vector rotate, double angle, double scroll_speed) {
    Vector cen = get_center();
    double scale = get_scale(cen);
    Vector pos_from_center = vec_multiply(scale, vec_subtract(pos, center));

    SDL_Rect placement = { cen.x + pos_from_center.x + scroll_speed * scroll_pos.x,
                           cen.y - pos_from_center.y + scroll_speed * scroll_pos.y,
                           size.x, size.y };
    if(centered) {
        placement.x -= size.x / 2;
        placement.y += size.y / 2;
    }
    SDL_Point rotater = {size.x * rotate.x, size.y * rotate.y};
    SDL_RenderCopyEx(renderer, image, NULL, &placement, angle * 180/M_PI, &rotater, SDL_FLIP_NONE);
}

void sdl_render_fixed_image(Vector pos, Vector size, SDL_Texture *image, bool centered) {
    sdl_render_image(vec_subtract(pos, scroll_pos), size, image, centered, 0);
}

void sdl_set_scroll(Vector amount) {
    scroll_vel = amount;
}

Vector sdl_get_scroll_speed(void) {
    return scroll_vel;
}

void sdl_set_zoom(double amount) {
    scroll_pos = vec_multiply(amount,scroll_pos);
    zoom_amount = amount;
}

void sdl_set_zoom_rate(double rate) {
    zoom_rate = rate;
}

void sdl_set_zoom_range(Vector range) {
    zoom_range = range;
}

void sdl_change_zoom(double amount, double limit) {
    if (fabs(zoom_amount + amount) < fabs(limit)) {
        zoom_amount += amount;
        scroll_pos = vec_add(vec_multiply(amount, scroll_pos), scroll_pos);
    }
}

void sdl_change_scroll(Vector amount, int range){
    if (fabs(amount.x + scroll_vel.x) < range){
        scroll_vel.x += amount.x;
    }

    if (fabs(amount.y + scroll_vel.y) < range){
        scroll_vel.y += amount.y;
    }
}

void sdl_show(void) {
    SDL_RenderPresent(renderer);
}

Vector get_size(List *shape) {
    size_t i;
    double x = 0, y = 0;
    Vector centroid = polygon_centroid(shape);
    for (i = 0; i < list_size(shape); i++) {
        Vector vertex = *((Vector *) list_get(shape, i));
        double dx = fabs(vertex.x - centroid.x);
        double dy = fabs(vertex.y - centroid.y);
        if (dx > x) {
            x = dx;
        }
        if (dy > y) {
            y = dy;
        }
    }
    return (Vector) { .x = 2 * x, .y = 2 * y };
}

void sdl_update_scroll(double dt) {
    if (zoom_amount + zoom_rate < zoom_range.y && zoom_amount + zoom_rate > zoom_range.x) {
        zoom_amount += zoom_rate;
    }
    Vector cen = get_center();
    cen = vec_multiply( 1/zoom_amount, vec_add(cen, scroll_pos));
    scroll_pos = vec_add(scroll_pos, vec_multiply(dt, scroll_vel));

    // Restrict zooming
    if (fabs(scroll_pos.x) <= fabs(4 * max_diff.x * (zoom_amount - 1))) {
        scroll_pos.x += dt * scroll_vel.x;
    } else {
        double overflow = fabs(4 * max_diff.x * (zoom_amount - 1)) - fabs(scroll_pos.x);
        scroll_pos.x += scroll_pos.x < 0 ? -1 * overflow : overflow;
    }
    if (fabs(scroll_pos.y) <= fabs(4 * max_diff.y * (zoom_amount - 1))) {
        scroll_pos.y += dt * scroll_vel.y;
    } else {
        double overflow = fabs(4 * max_diff.y * (zoom_amount - 1)) - fabs(scroll_pos.y);
        scroll_pos.y += scroll_pos.y < 0 ? -1 * overflow : overflow;
    }
}

void sdl_render_scene(Scene *scene, List *textures) {
    sdl_clear();
    size_t i;
    size_t body_count = scene_bodies(scene);
    List *strings = scene_strings(scene);
    Vector cen = get_center();
    double scale = zoom_amount * (cen.y / max_diff.y);
    for (i = 0; i < list_size(scene->backgrounds); i++) {
        Background *background = (Background *) list_get(scene->backgrounds, i);
        Body *sb = background->back;
        // Have to transform vector because of SDL coordinates
        sdl_render_image(vec_add(body_get_centroid(sb), (Vector){0, 2 * max_diff.y}), vec_multiply(scale, sb->size), 
            sb->texture, true, sb->ss);
    }
    for (i = 0; i < body_count; i++) {
        Body *body = scene_get_body(scene, i);
        if (body->texture_id >= 0) {
            SDL_Texture *texture = tileset_get_texture(textures, (size_t) body->texture_id);
            sdl_render_rotated_image(body_get_centroid(body), vec_multiply(scale * 1.05, body->size),
                texture, true, (Vector) {0.5, 0.5}, body->angle * -1, body->ss);
            if(body->damage_overlay != NULL){
                // SDL_Texture *damage = body->damage_overlay;
                sdl_render_rotated_image(body_get_centroid(body), vec_multiply(scale * 1.05, body->size),
                    body->damage_overlay, true, (Vector) {0.5, 0.5}, body->angle * -1, body->ss);
            }
        } else if (body->texture != NULL) {
            sdl_render_rotated_image(vec_add(body_get_centroid(body), (Vector){0, body->size.y}),
                vec_multiply(scale, body->size), body->texture, true, (Vector) {0.5, 0.5}, body->angle * -1, body->ss);
        } else {
            List *shape = body_get_shape(body);
            sdl_draw_polygon(shape, body_get_color(body));
        }
        // list_free(shape);
    }
    for (i = 0; i < list_size(strings); i++) {
        Text *text = (Text *) list_get(strings, i);
        if (text->fixed) {
            sdl_render_fixed_text(text->position, text->string, text->color);
        } else {
            sdl_render_text(text->position, text->string, text->color);
        }
    }
    sdl_show();
}

void sdl_on_key(KeyHandler handler, void *data) {
    key_data = data;
    key_handler = handler;
}

void sdl_on_mouse(MouseHandler handler, void *data) {
    mouse_data = data;
    mouse_handler = handler;
}

double time_since_last_tick(void) {
    clock_t now = clock();
    double difference = last_clock
        ? (double) (now - last_clock) / CLOCKS_PER_SEC
        : 0.0; // return 0 the first time this is called
    last_clock = now;
    return difference;
}

void sdl_destroy(void) {
    TTF_Quit();
    IMG_Quit();
    Mix_Quit(); // _AUDIO_
    SDL_DestroyWindow(window);
}

////////////////////////////////////////////////////////
// _AUDIO_ BELOW comment out for compiling on compute //
////////////////////////////////////////////////////////

Mix_Chunk *load_audio_chunk(char *audio_path){
    Mix_Chunk *temp = Mix_LoadWAV(audio_path);
    if(temp == NULL){
        printf("Failed to load %s\n%s\n", audio_path, Mix_GetError());
        exit(1);
    }
    return temp;
}

Mix_Music *load_music(char *audio_path){
    Mix_Music *temp =  Mix_LoadMUS(audio_path);
    if (temp == NULL){
        printf("Failed to load %s\n%s\n", audio_path, Mix_GetError());
        exit(1);
    }
    return temp;
}

int play_audio_chunk(Mix_Chunk *chunk, int times){
    return Mix_PlayChannel(-1, chunk, times); // -1 puts it on the first open channel 
}

void stop_channel(int channel, int time){
    Mix_ExpireChannel(channel, 100);
}

void play_music(Mix_Music *music){
    Mix_PlayMusic(music, -1); // -1 indicates infinate looping
}

void pause_music(){
    Mix_PauseMusic();
}

void resume_music(){
    Mix_ResumeMusic();
}
