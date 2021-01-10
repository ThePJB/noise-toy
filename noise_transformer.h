#pragma once

#include <stdint.h>
#include <cglm/struct.h>

#include "noise.h"

typedef struct {
    mat3s input;
    mat4s output;
    mat4s output_inv;
    mat4s normal_fixer;
} noise_transformer;

typedef struct {
    vec3s pos;
    vec3s normal;
} pos_normal;

noise_transformer noise_tformer_new(mat3s input, mat4s output);
pos_normal noise_tformer_sample(noise_transformer nt, noise2d_func f, float x, float y, uint32_t seed);
pos_normal noise_tformer_sample_from_output(noise_transformer nt, noise2d_func f, float x, float y, uint32_t seed);