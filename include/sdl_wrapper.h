#ifndef __SDL_WRAPPER_H__
#define __SDL_WRAPPER_H__


#include <SDL2/SDL.h>
#include <SDL2/SDL_render.h>
#include <SDL2/SDL2_gfxPrimitives.h>
#include <SDL2/SDL_ttf.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_mixer.h> // _AUDIO_

#include <assert.h>
#include <math.h>
#include <stdlib.h>
#include <time.h>
#include <stdbool.h>
#include "color.h"
#include "list.h"
#include "scene.h"
#include "vector.h"
#include "tileset.h"

// Values passed to a key handler when the given arrow key is pressed
#define LEFT_ARROW 1
#define UP_ARROW 2
#define RIGHT_ARROW 3
#define DOWN_ARROW 4
#define SPACEBAR 5
#define EQUALS 6
#define MINUS 7
#define TAB 8

/**
 * The possible types of key events.
 * Enum types in C are much more primitive than in Java; this is equivalent to:
 * typedef unsigned int KeyEventType;
 * #define KEY_PRESSED 0
 * #define KEY_RELEASED 1
 */
typedef enum {
    KEY_PRESSED,
    KEY_RELEASED
} KeyEventType;

typedef enum {
    MOUSEMOTION,
    MOUSEBUTTONDOWN,
    MOUSEBUTTONUP
} MouseEventType;

/**
 * A keypress handler.
 * When a key is pressed or released, the handler is passed its char value.
 * Most keys are passed as their char value, e.g. 'a', '1', or '\r'.
 * Arrow keys have the special values listed above.
 *
 * @param key a character indicating which key was pressed
 * @param type the type of key event (KEY_PRESSED or KEY_RELEASED)
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*KeyHandler)(char key, KeyEventType type, double held_time, void *data);

/**
 * A mouse event handler.
 * When the mouse is moved/clicked/released this is called
 *
 * @param key a character indicating which key was pressed
 * @param type the type of mouse event 0:mousemove, 1:mouseclick, 2:mouserelease
 * @param held_time if a press event, the time the key has been held in seconds
 */
typedef void (*MouseHandler)(int x, int y, MouseEventType type, void *data);

/**
 * Initializes the SDL window and renderer.
 * Must be called once before any of the other SDL functions.
 *
 * @param min the x and y coordinates of the bottom left of the scene
 * @param max the x and y coordinates of the top right of the scene
 */
void sdl_init(Vector min, Vector max);//, char *path);

/**
 * set the font that the renderer will use
 *
 * @param font_path the path to the font
 * @param size the size of the font
 */
// void set_font(char *font_path, size_t size);

/**
 * handles all mouse events
 *
 * @param x the x coord of the event
 * @param y the y coord of the event
 * @param type the tyoe of the event: 0:mousemove, 1:mouseclick, 2:mouserelease
 */
void mouse_event(int x, int y, MouseEventType type);

/**
 * Processes all SDL events and returns whether the window has been closed.
 * This function must be called in order to handle keypresses.

 *
 * @return true if the window was closed, false otherwise
 */
bool sdl_is_done(void);

/**
 * Clears the screen. Should be called before drawing polygons in each frame.
 */
void sdl_clear(void);

/**
 * get the center of the window
 */
Vector get_center(void);

/**
 * Scale scene so it fits entirely in the window,
 * with the center of the scene at the center of the window
 */
double get_scale(Vector center);

/**
 * Draws a polygon from the given list of vertices and a color.
 *
 * @param points the list of vertices of the polygon
 * @param color the color used to fill in the polygon
 */
void sdl_draw_polygon(List *points, RGBColor color);
/**
* Renders a given string to the texture supplied
*
* @param text the text to render
* @param texture The SDL_Texture to render to
* @param pos the top left corner of the text
*/
void sdl_render_text(Vector pos, char *text, RGBColor color);

void sdl_render_fixed_text(Vector pos, char *text, RGBColor color);

/**
 * loads an image from the given path (use .bmp)
 *
 * @param image_path the path to the image
 */
SDL_Texture *sdl_load_image(char *image_path);

/**
 * draws an image at the pos marked with the supplied width
 *
 * @param pos the place where the image will be drawn (lop left corner)
 * @param size the width, height of the image
 * @param image the texture which the image has been 'loaded' to
 * @param center if you want to draw from the center of the image set true
 */
void sdl_render_image(Vector pos, Vector size, SDL_Texture *image, bool centered, double scroll_speed);

void sdl_render_fixed_image(Vector pos, Vector size, SDL_Texture *image, bool centered);

/**
 * draws an image at the pos marked with the supplied width
 *
 * @param pos the place where the image will be drawn (lop left corner)
 * @param size the width, height of the image
 * @param image the texture which the image has been 'loaded' to
 * @param center if you want to draw from the center of the image set true
 * @param rotate the unit vector at which to rotate around
 * @param angle, the amount of rotation
 */
void sdl_render_rotated_image(Vector pos, Vector size, SDL_Texture *image, bool centered, Vector rotate, double angle, double scroll_speed);

/**
 * scrolls the screen by the given amount
 *
 * @param amount the amount of scrolling;
 */
void sdl_set_scroll(Vector amount);

/**
 * get the scroll vector
 */
Vector sdl_get_scroll_speed(void);

/**
 * set the zoom of sdl
 *
 * @param amount the value to set the zoom to
 */
void sdl_set_zoom(double amount);

void sdl_change_zoom(double amount, double limit);

void sdl_set_zoom_rate(double rate);

void sdl_set_zoom_range(Vector range);

/**
 * changes the scroll vel by the given amount if it is within the range provided
 *
 * @param amount the amount to change by
 * @param range the range we are checking
 */
void sdl_change_scroll(Vector amount, int range);

/**
 * Displays the rendered frame on the SDL window.
 * Must be called after drawing the polygons in order to show them.
 */
void sdl_show(void);

/**
 * update the scrolling enviroment
 *
 * @param dt the timestep since the last call of this function
 */
void sdl_update_scroll(double dt);

/**
 * Draws all bodies in a scene.
 * This internally calls sdl_clear(), sdl_draw_polygon(), and sdl_show(),
 * so those functions should not be called directly.
 *
 * @param scene the scene to draw
 */
void sdl_render_scene(Scene *scene, List *textures);

// /**
// * set the font that will be used in this window when rendering text
// *
// * @param path path to the .ttf file
// * @param size the font_size
// */
void sdl_set_font(char* path, size_t size);

/**
 * Registers a function to be called every time a key is pressed.
 * Overwrites any existing handler.
 *
 * Example:
 * ```
 * void on_key(char key, KeyEventType type, double held_time) {
 *     if (type == KEY_PRESSED) {
 *         switch (key) {
 *             case 'a':
 *                 puts("A pressed");
 *                 break;
 *             case UP_ARROW:
 *                 puts("UP pressed");
 *                 break;
 *         }
 *     }
 * }
 * int main(int argc, char **argv) {
 *     sdl_on_key(on_key);
 *     while (!sdl_is_done());
 * }
 * ```
 *
 * @param handler the function to call with each key press
 * @param data the aux data
 */
void sdl_on_key(KeyHandler handler, void *data);

/**
 * Works just like sdl_on_key
 *
 * @param handler the function to call with each mouse event
 * @param data the aux data
 */
void sdl_on_mouse(MouseHandler handler, void *data);

/**
 * Gets the amount of time that has passed since the last time
 * this function was called, in seconds.
 *
 * @return the number of seconds that have elapsed
 */
double time_since_last_tick(void);

void sdl_destroy(void);

////////////////////////////////////////////////////////
// _AUDIO_ BELOW comment out for compiling on compute //
////////////////////////////////////////////////////////

/**
 * loads the given audio chunk (please use .wav)
 *
 * to load the chunk use this:
 * Mix_Chunk example = load_audio_chunk(path)
 * store the chunk in the main driver so it does not have to be reloaded
 * free it with Mix_FreeChunk(example)
 *
 * @param audio_path the path to the audio chunk
 */
Mix_Chunk *load_audio_chunk(char *audio_path);

/**
 * loads the given music (please use .wav)
 *
 * to load Music use this:
 * Mix_Music example = load_music(path)
 * free it with Mix_FreeMusic(eexample)
 *
 * @param audio_path the path to the audio chunk
 */
Mix_Music *load_music(char *audio_path);

/**
 * play the given audio chunk
 * 
 * returns the channel that the sound is played on
 *
 * @param chunk the chunk to be played
 * @param times, the number of times the chunk will loop (-1 for infinate)
 */
int play_audio_chunk(Mix_Chunk *chunk, int times);

/**
 * stop an audio channel
 * 
 * @param channel the number that indicates the channel the music is on (-1 stops all)
 * @param time the amount of time in milliseconds until the sound is halted.
 */
void stop_channel(int channel, int time);

/**
 * play the given music on loop
 *
 * @param music the music to play
 */
void play_music(Mix_Music *music);

/**
 * pauses music played with play_music(Mix_Music *music)
 */
void pause_music(void);

/**
 * resumes music that was previously paused
 */
void resume_music(void);


#endif // #ifndef __SDL_WRAPPER_H__
