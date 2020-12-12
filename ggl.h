#include <SDL.h>
#include <stdio.h>

#define ggl_die(C, MSG) printf("%s %d %s: dying -- %s -- %s\n", __FILE__, __LINE__, __func__, MSG, SDL_GetError()), ggl_teardown(C)

typedef struct {
    SDL_Window *window;
    int xres;
    int yres;
    SDL_GLContext gl;
} gg_context;

void ggl_teardown(gg_context *g);
gg_context *ggl_init(char *title);
