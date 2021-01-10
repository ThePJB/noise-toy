#pragma once

#include "ggl.h"
#include <cglm/struct.h>

typedef struct {
    vec3s pos;
    vec3s normal;
    vec3s colour;
} PNC_Vert;

typedef PNC_Vert PNC_Tri[3];

typedef struct {
    size_t backing_length;
    size_t num_tris;
    PNC_Tri *tris;
    
    vao vao;
    vbo vbo;

    mat4s model;

    bool uploaded;

} PNC_Mesh;

PNC_Mesh pnc_new(mat4s model);

void pnc_push_tri(PNC_Mesh *m, PNC_Vert v1, PNC_Vert v2, PNC_Vert v3, mat4s transform);

void pnc_push_trunc_cone(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s top_pos,
        vec3s top_axis,
        vec3s bot_pos,
        vec3s bot_axis,
        float r_top,
        float r_bot,
        int num_sides,
        mat4s transform);

/*
void pnc_push_cone(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s top_pos,
        //vec3s top_axis,
        vec3s bot_pos,
        vec3s bot_axis,
        float r_bot,
        int num_sides);
*/

void pnc_push_ellipsoid(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s center,
        vec3s axis,
        float d,
        float h,
        int circ_sides,
        int h_sides,
        mat4s transform);


void pnc_upload(PNC_Mesh *m);
void pnc_draw(PNC_Mesh m, shader_pgm_id pgm, float *view, float *proj);

void pnc_print_summary(PNC_Mesh m);
void pnc_print_tris(PNC_Mesh m);

void pnc_free_ram(PNC_Mesh m);
void pnc_free_vram(PNC_Mesh m);

PNC_Mesh pnc_test_mesh();