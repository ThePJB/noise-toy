#include <SDL.h>
#include <stdio.h>
#include <cglm/struct.h>

#define ggl_die(C, MSG) printf("%s %d %s: dying -- %s -- %s\n", __FILE__, __LINE__, __func__, MSG, SDL_GetError()), ggl_teardown(C)

typedef struct {
    SDL_Window *window;
    int xres;
    int yres;
    SDL_GLContext gl;
} gg_context;

typedef struct {
    vec3s pos;
    vec3s normal;
    vec3s colour;
} PNC_Vert;

typedef PNC_Vert PNC_Tri[3];

typedef struct {
    int num_tris;
    PNC_Tri *tris;
} PNC_Mesh;

typedef unsigned int shader_id;
typedef unsigned int shader_pgm_id;

void ggl_teardown(gg_context *g);
gg_context *ggl_init(char *title, int xres, int yres);
shader_id ggl_make_shader(gg_context *g, char *program, unsigned int type);
shader_pgm_id ggl_make_shader_pgm(gg_context *g, shader_id vertex_shader, shader_id fragment_shader);

typedef unsigned int vao;
typedef unsigned int vbo;

vao ggl_upload_pnc(PNC_Mesh m);