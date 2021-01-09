#include <cglm/struct.h>
#include <stdarg.h>
#include "glad.h"
#include "ggl.h"

// good graphics library

void ggl_teardown(gg_context *g) {
    SDL_DestroyWindow(g->window);
    SDL_Quit();
}

gg_context *ggl_init(char *title, int xres, int yres) {
    printf("Initializing ggl...\n");
    gg_context *g = calloc(1, sizeof(gg_context));
    g->xres = xres;
    g->yres = yres;

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
    //glDisable(GL_DEPTH_TEST);
    glEnable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    // if in fullscreen mode we need to ask res
    SDL_GetWindowSize(g->window, &g->xres, &g->yres);
    glViewport(0,0,g->xres, g->yres);
    glClearColor(0.0f, 0.5f, 1.0f, 0.0f);

    return g;
}

shader_id ggl_make_shader(gg_context *g, char *program, unsigned int type) {
    unsigned int shader = glCreateShader(type);
    glShaderSource(shader, 1, (const GLchar * const*)&program, NULL);
    glCompileShader(shader);
    int success = 0;
    char buf[512] = {0};
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);

    if (!success) {
        glGetShaderInfoLog(shader, 512, NULL, buf);
        printf("error compiling: %s\n", buf);
        ggl_die(g, "couldn't compile shader");
    }

    return (shader_id)shader;
}

shader_pgm_id ggl_make_shader_pgm(gg_context *g, shader_id vertex_shader, shader_id fragment_shader) {    
    int success;
    char buf[512] = {0};
    shader_pgm_id shader_program;
    shader_program = glCreateProgram();
    glAttachShader(shader_program, vertex_shader);
    glAttachShader(shader_program, fragment_shader);
    glLinkProgram(shader_program);

    glGetProgramiv(shader_program, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shader_program, 512, NULL, buf);
        printf("error linking shaders: %s\n", buf);
        ggl_die(g, "couldn't make shader program");
    }

    glDeleteShader(vertex_shader);
    glDeleteShader(fragment_shader);
    return shader_program;
}

