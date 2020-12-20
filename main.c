#include <SDL.h>
#include <cglm/struct.h>
#include <stdbool.h>
#include <math.h>
#include "ggl.h"
#include "mymath.h"
#include "util.h"
#include "glad.h"
#include "noise.h"
#include "camera.h"

PNC_Vert make_vert(float x, float y, float z, vec3s normal,
                float xscale, float yscale, float zscale) {
    const vec3s low_colour = {0.2, 0.5, 0};
    const vec3s high_colour = {0.5, 0.5, 0};

    return (PNC_Vert) {
            .pos = {x*xscale, y*yscale, z*zscale},
            .normal = normal, // todo maybe normalize for the scale
            .colour = glms_vec3_lerp(low_colour, high_colour, y),
    };
}

// alloc and make a mesh
// todo replace y and z
PNC_Mesh generate_mesh(float (*noise_func)(float x, float y, uint32_t seed),
        uint32_t seed,
        float startx, float endx, int xsamples,
        float starty, float endy, int ysamples,
        float xscale, float yscale, float zscale) {

    float stepx = (endx - startx) / xsamples;
    float stepy = (endy - starty) / ysamples;

    int num_quads = xsamples * ysamples;

    int num_tris = num_quads * 2;
    PNC_Mesh m = {0};
    m.num_tris = num_tris;
    m.tris = calloc(num_tris, sizeof(PNC_Tri));
    int tris_index = 0;
    for (int i = 0; i < xsamples; i++) {
        for (int j = 0; j < ysamples; j++) {
            //printf("tri index: %d i: %d j: %d\n", tris_index, i, j);
            float x0 = startx + i*stepx;
            float x1 = startx + (i+1)*stepx;
            float y0 = starty + j*stepy;
            float y1 = starty + (j+1)*stepy;
            
            float h00 = noise_func(x0, y0, seed);
            float h01 = noise_func(x0, y1, seed);
            float h10 = noise_func(x1, y0, seed);
            float h11 = noise_func(x1, y1, seed);

            // make normal (todo analytical normal)

            // colour = whatever
            // normal = from pos verts
            // pos verts = x0,y0,v00, x0,y1,v01, etc
            // also winding order

            vec3s normal_low = normal_from_verts(
                (vec3s) {x0, h00, y0},
                (vec3s) {x1, h10, y0},
                (vec3s) {x0, h01, y1}
            );
            
            vec3s normal_high = normal_from_verts(
                (vec3s) {x1, h10, y0},
                (vec3s) {x1, h11, y1},
                (vec3s) {x0, h01, y1}
            );

            m.tris[tris_index][0] = make_vert(x0, h00, y0, normal_low, xscale, yscale, zscale);
            m.tris[tris_index][1] = make_vert(x1, h10, y0, normal_low, xscale, yscale, zscale);
            m.tris[tris_index][2] = make_vert(x0, h01, y1, normal_low, xscale, yscale, zscale);
            tris_index++;
            
            m.tris[tris_index][0] = make_vert(x1, h10, y0, normal_low, xscale, yscale, zscale);
            m.tris[tris_index][1] = make_vert(x1, h11, y1, normal_low, xscale, yscale, zscale);
            m.tris[tris_index][2] = make_vert(x0, h01, y1, normal_low, xscale, yscale, zscale);
            tris_index++;
        }
    }
    return m;
}

typedef enum {
    NM_FRACTAL,
    NM_DOMWARP_1,
    NM_DOMWARP_2,
    NM_DOMWARP_3,
    NM_RIDGE,
    NM_BILLOW,
    NUM_NM,
} noise_mode;

typedef float(*noise2d_func)(float, float, uint32_t);

noise2d_func noise_funcs[NUM_NM] = {
    fbm2_bilinear,
    fbm2_bilinear_domwarp1,
    fbm2_bilinear_domwarp2,
    fbm2_bilinear_domwarp3,
    billow,
    ridge,
};

char *nm_name[NUM_NM] = {
    "fractal",
    "domwarp 1",
    "domwarp 2",
    "domwarp 3",
    "ridge",
    "billow",
};

noise_mode current_noise_mode = 0;


vao nm_vao[NUM_NM] = {0};
PNC_Mesh nm_mesh[NUM_NM] = {0};

void switch_terrain(noise_mode nm, uint32_t seed) {
    current_noise_mode = nm;
    printf("switched to %s\n", nm_name[nm]);
    if (!nm_mesh[nm].tris) {
        // if we need to generate this one
        nm_mesh[nm] = generate_mesh(noise_funcs[nm], seed,
            -5, 5, 1000,
            -5, 5, 1000,
            10, 5, 10
        );
        nm_vao[nm] = ggl_upload_pnc(nm_mesh[nm]);
    }
}

int main(int argc, char** argv) {
    int xres = 1920;
    int yres = 1080;


    gg_context *g = ggl_init("nxplore", xres, yres);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    float t = 0;

    // make shaders
    char *vert = slurp("heightmap.vert");
    char *frag = slurp("heightmap.frag");

    printf("heightmap.vert\n");
    shader_id vert_id = ggl_make_shader(g, vert, GL_VERTEX_SHADER);
    printf("heightmap.frag\n");
    shader_id frag_id = ggl_make_shader(g, frag, GL_FRAGMENT_SHADER);

    shader_pgm_id heightmap_pgm = ggl_make_shader_pgm(g, vert_id, frag_id);
    free(vert);
    free(frag);

    camera cam = fly_camera();
    
    mat4s view = GLMS_MAT4_IDENTITY_INIT;
    mat4s proj = GLMS_MAT4_IDENTITY_INIT;

    uint32_t seed = 123456;
    switch_terrain(0, seed);


    bool do_2d = true;
    bool keep_going = true;
    while(keep_going) {
        // update cam matrices
        view = glms_lookat(
            cam.pos, 
            glms_vec3_add(cam.pos, cam.front),
            cam.up
        );

        proj = glms_perspective(
            glm_rad(cam.fovx), 
            (float)xres / (float)yres, 0.1, 10000
        );

        // Handle Input
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) keep_going = false;
            if (e.type == SDL_KEYDOWN) {
                SDL_Keycode sym = e.key.keysym.sym;
                if (sym == SDLK_ESCAPE) {
                    keep_going = false;
                } else if (sym == SDLK_r) {
                    seed = squirrel3(seed, 1);
                    printf("Reseed %u\n", seed);
                } else if (sym == SDLK_SPACE) {
                    do_2d = !do_2d;
                } else if (sym >= SDLK_1 && sym <= SDLK_9) {
                    int nm = sym - SDLK_1;
                    if (nm < NUM_NM) {
                        switch_terrain(nm, seed);                       
                    }
                }
            } else if (e.type == SDL_MOUSEMOTION) {
                cam = camera_update(cam, e.motion.xrel, e.motion.yrel);
            }
        }

        const uint8_t *state = SDL_GetKeyboardState(NULL);
        float movement_speed = 0.1;
        if (state[SDL_SCANCODE_W]) {
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.front, movement_speed));
        }
        if (state[SDL_SCANCODE_S]) {
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.front, -1 * movement_speed));
        }
        if (state[SDL_SCANCODE_A]) {
            vec3s cam_left = glms_normalize(glms_vec3_cross(cam.up, cam.front));
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam_left, movement_speed));
        }
        if (state[SDL_SCANCODE_D]) {
            vec3s cam_left = glms_normalize(glms_vec3_cross(cam.up, cam.front));
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam_left, -1 * movement_speed));
        }
        if (state[SDL_SCANCODE_SPACE]) {
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.up, movement_speed));
        }
        if (state[SDL_SCANCODE_LSHIFT]) {
            cam.pos = glms_vec3_add(cam.pos, glms_vec3_scale(cam.up, -1 * movement_speed));
        }

        glClearColor(0.3, 0.5, 0.7, 1);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        glUseProgram(heightmap_pgm);
        glUniformMatrix4fv(glGetUniformLocation(heightmap_pgm, "view"), 1, GL_FALSE, view.raw[0]);
        glUniformMatrix4fv(glGetUniformLocation(heightmap_pgm, "proj"), 1, GL_FALSE, proj.raw[0]);
        glBindVertexArray(nm_vao[current_noise_mode]);
        glDrawArrays(GL_TRIANGLES, 0, nm_mesh[current_noise_mode].num_tris * 3); // handle should probably contain num triangles

        SDL_GL_SwapWindow(g->window);
    }

    ggl_teardown(g);
}

void frame(double dt) {

}