#include <cglm/struct.h>
#include "glad.h"
#include "ggl.h"



// good graphics library
// to replace my name gef



void ggl_teardown(gg_context *g) {
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

gg_context *ggl_init(char *title) {
    printf("Initializing ggl...\n");
    gg_context *g = calloc(1, sizeof(gg_context));
    g->xres = 640;
    g->yres = 480;

    if (SDL_Init(SDL_INIT_VIDEO) < 0) ggl_die(g, "couldn't init SDL\n");
    SDL_GL_LoadLibrary(NULL);
    SDL_GL_SetAttribute(SDL_GL_ACCELERATED_VISUAL, 1);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    g->window = SDL_CreateWindow(
        title,
        SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
        g->xres, g->yres, SDL_WINDOW_OPENGL
    );

    if (g->window == NULL) {
        ggl_die(g, "couldn't create window\n");
    }

    g->gl = SDL_GL_CreateContext(g->window);
    if (g->window == NULL) {
        ggl_die(g, "couldn't create opengl context\n");
    }

    printf("OpenGL loaded\n");
    gladLoadGLLoader(SDL_GL_GetProcAddress);
    printf("Vendor:   %s\n", glGetString(GL_VENDOR));
    printf("Renderer: %s\n", glGetString(GL_RENDERER));
    printf("Version:  %s\n", glGetString(GL_VERSION));

    SDL_GL_SetSwapInterval(1);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // if in fullscreen mode we need to ask res
    SDL_GetWindowSize(g->window, &g->xres, &g->yres);
    glViewport(0,0,g->xres, g->yres);
    glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

    return g;
}