#pragma once

#include <stdint.h>

#include "ggl.h"
#include "pnc.h"
#include "camera.h"
#include "noise_transformer.h"
#include "noise.h"
#include "alpha_noise.h"
#include "proc_tree.h"
#include "world.h"

typedef enum {
    NM_ALPHA,
    NUM_NM,
} noise_mode;

typedef struct {
    gg_context *g;
    camera cam;
    shader_pgm_id heightmap_pgm;
    
    uint32_t current_seed;
    noise2d_func current_noise_func;
    noise_mode current_noise_mode;
    PNC_Mesh terrain[NUM_NM];

    noise_transformer tnt;

    bool keep_going;
} application;



void handle_input(application *app, double dt);
void draw(application *app);

application application_init(int xres, int yres);