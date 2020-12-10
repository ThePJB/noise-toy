#include <stdlib.h>
#include <SDL.h>
#include <SDL_image.h>
#include "gef.h"

typedef struct { 
    SDL_Renderer *renderer;
    SDL_Window *window;
    SDL_Texture *atlas;
    int xres;
    int yres;
} gef;

gef gef_context = {0};

void gef_set_name(char *name) {
    SDL_SetWindowTitle(gef_context.window, name);
}

void gef_set_res(int x, int y) {
    gef_context.xres = x;
    gef_context.yres = y;
    SDL_SetWindowSize(gef_context.window, x, y);
}

int gef_get_xres() {
    return gef_context.xres;
}

int gef_get_yres() {
    return gef_context.yres;
}

void gef_init() {
    printf("initializing graphics...\n");
    gef *g = calloc(1, sizeof(gef));

    gef_context.xres = 640;
    gef_context.yres = 480;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) gef_die("couldn't init sdl");

    gef_context.window = SDL_CreateWindow("", 
        SDL_WINDOWPOS_UNDEFINED, 
        SDL_WINDOWPOS_UNDEFINED, 
        gef_context.xres, 
        gef_context.yres,
        SDL_WINDOW_SHOWN);

    if (gef_context.window == NULL) gef_die("couldn't create window");

    gef_context.renderer = SDL_CreateRenderer(gef_context.window, -1, SDL_RENDERER_ACCELERATED);
    if (gef_context.renderer == NULL) gef_die("couldn't create renderer");

    //if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG);
    /* load assets */
    /*
    SDL_Surface* loaded_surface = IMG_Load("assets/atlas.png");
    g.atlas = SDL_CreateTextureFromSurface(g.renderer, loaded_surface);
    if (g.atlas == NULL) die("couldn't create texture");
    SDL_FreeSurface(loaded_surface);
    */
}

void gef_put_pixel(int x, int y, int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(gef_context.renderer, r, g, b, a);
    SDL_RenderDrawPoint(gef_context.renderer, x, y);
}

void gef_clear() {
    SDL_RenderClear(gef_context.renderer);
}

void gef_set_colour(int r, int g, int b, int a) {
    SDL_SetRenderDrawColor(gef_context.renderer, 0, 0, 0, 255);
}

void gef_present() {
    SDL_RenderPresent(gef_context.renderer);
}

void gef_teardown() {
    SDL_DestroyRenderer(gef_context.renderer);
    SDL_DestroyWindow(gef_context.window);
    IMG_Quit();
    SDL_Quit();
}