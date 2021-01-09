#pragma once

#include "cglm/struct.h"
#include "ggl.h"

typedef struct {
    vec3s a;
    vec3s b;
} line_segment;

typedef struct {
    line_segment *lines;
    size_t backing_length;
    size_t num_segs;

    vao vao;
    bool uploaded;
    
} line_mesh;

void line_upload(line_mesh *m);
void line_draw(line_mesh m, shader_pgm_id pgm, float *view, float *proj);
line_mesh line_new();
void line_push(line_mesh *m, vec3s v1, vec3s v2);