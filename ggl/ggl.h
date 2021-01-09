#pragma once

#include <SDL.h>
#include <stdio.h>
#include <cglm/struct.h>

#define ggl_die(C, MSG) printf("%s %d %s: dying -- %s -- %s\n", __FILE__, __LINE__, __func__, MSG, SDL_GetError()), ggl_teardown(C)

typedef unsigned int shader_id;
typedef unsigned int shader_pgm_id;

typedef unsigned int vao;
typedef unsigned int vbo;

typedef struct {
    SDL_Window *window;
    int xres;
    int yres;
    SDL_GLContext gl;
} gg_context;

void ggl_teardown(gg_context *g);
gg_context *ggl_init(char *title, int xres, int yres);
shader_id ggl_make_shader(gg_context *g, char *program, unsigned int type);
shader_pgm_id ggl_make_shader_pgm(gg_context *g, shader_id vertex_shader, shader_id fragment_shader);


