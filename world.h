#pragma once

#include <cglm/struct.h>

#include "pnc.h"
#include "noise_transformer.h"
#include "noise.h"
#include "proc_tree.h"
#include "util.h"

PNC_Vert make_heightmap_vert(vec3s pos, vec3s normal);

// x and y in 
void push_tree(PNC_Mesh *m, noise_transformer nt, noise2d_func f, float x, float z, l_tree_params ltp, uint32_t world_seed);

void push_terrain(PNC_Mesh *m, noise2d_func noise_func, uint32_t seed, int xsamples, int zsamples,
        noise_transformer nt,
        mat4s transform);