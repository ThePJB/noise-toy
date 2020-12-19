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



// todo smooth it out (kernel interp etc)
// then do gradient noise, see how fast it can get and if it look good


float quant_err(float x, float y, uint32_t seed) {
    return (x - fastfloor(x) + y - fastfloor(y)) / 2;
}

// bugged
float quant_err3(float x, float y, float z, uint32_t seed) {
    return frac(x) + frac(y) + frac(z) / 3;
}



float fbm2_bilinear_domwarp1(float x, float y, uint32_t seed) {
    const float warp_coeff = 1;
    return fbm2_bilinear(x + warp_coeff * fbm2_bilinear(x, y, seed+69), y + warp_coeff * fbm2_bilinear(x,y,seed+420), seed);
}

float fbm3_domwarp1(float x, float y, float z, uint32_t seed) {
    const float warp_coeff = 1;
    return fbm3(x + warp_coeff * fbm3(x, y, z, seed+69), y + warp_coeff * fbm3(x,y,z,seed+420), z + warp_coeff * fbm3(x,y,z, seed*12121), seed);
}

float fbm2_bilinear_domwarp2(float x, float y, uint32_t seed) {
    const float warp_coeff = 1;
    return fbm2_bilinear(x + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1337), y + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1919), seed);
}

float fbm2_bilinear_domwarp3(float x, float y, uint32_t seed) {
    const float warp_coeff = 1;
    return fbm2_bilinear_domwarp1(x + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1337), y + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1919), seed);
}

typedef struct {
    float startx;
    float starty;
    float wx;
    float wy;
} transform_2d;

/*
void draw_noise(float (*noise_func)(float x, float y, uint32_t seed), transform_2d t, uint32_t seed) {
    //int xres = gef_get_xres();
    //int yres = gef_get_yres();

    const float max = 2;

    for (int i = 0; i < xres; i++) {
        for (int j = 0; j < yres; j++) {
            float x = t.startx + ((float)i / (float)xres) * t.wx;
            float y = t.starty + ((float)j / (float)yres) * t.wy;

            float noise_sample = noise_func(x, y, seed) / max;
            gef_put_pixel(i, j, 255 * noise_sample, 255 * noise_sample, 255 * noise_sample, 255);
        }
    }
}

void draw_3d_noise_time(float (*noise_func)(float x, float y, float z, uint32_t seed), transform_2d tform, uint32_t seed, float t) {
    int xres = gef_get_xres();
    int yres = gef_get_yres();

    const float max = 2;

    for (int i = 0; i < xres; i++) {
        for (int j = 0; j < yres; j++) {
            float x = tform.startx + ((float)i / (float)xres) * tform.wx;
            float y = tform.starty + ((float)j / (float)yres) * tform.wy;

            float noise_sample = noise_func(x, y, t, seed) / max;
            gef_put_pixel(i, j, 255 * noise_sample, 255 * noise_sample, 255 * noise_sample, 255);
        }
    }   
}

void draw_noise_heightmap(float (*noise_func)(float x, float y, uint32_t seed), transform_2d t, uint32_t seed) {

}
*/

vec3s normal_from_verts(vec3s a, vec3s b, vec3s c) {
    vec3s ab = glms_vec3_sub(b,a);
    vec3s bc = glms_vec3_sub(c,a);
    return glms_vec3_normalize(glms_vec3_cross(bc, ab));
}

PNC_Vert make_vert(float x, float y, float z, vec3s normal) {
    const vec3s low_colour = {0.2, 0.5, 0};
    const vec3s high_colour = {0.5, 0.5, 0};

    return (PNC_Vert) {
            .pos = {x, y, z},
            .normal = normal,
            .colour = glms_vec3_lerp(low_colour, high_colour, y),
    };
}

// alloc and make a mesh
PNC_Mesh generate_mesh(float (*noise_func)(float x, float y, uint32_t seed),
        uint32_t seed,
        float startx, float endx, int xsamples,
        float starty, float endy, int ysamples) {

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

            m.tris[tris_index][0] = make_vert(x0, h00, y0, normal_low);
            m.tris[tris_index][1] = make_vert(x1, h10, y0, normal_low);
            m.tris[tris_index][2] = make_vert(x0, h01, y1, normal_low);
            tris_index++;
            
            m.tris[tris_index][0] = make_vert(x1, h10, y0, normal_low);
            m.tris[tris_index][1] = make_vert(x1, h11, y1, normal_low);
            m.tris[tris_index][2] = make_vert(x0, h01, y1, normal_low);
            tris_index++;
        }
    }
    return m;
}

typedef enum {
    NM_DOMWARP_3,
    NM_RIDGE,
    NM_BILLOW,
    NUM_NM,
} noise_mode;

//PNC_Mesh meshes[NUM_NM] = {0};

typedef float(*noise2d_func)(float, float, uint32_t);

noise2d_func noise_funcs[NUM_NM] = {
    fbm2_bilinear_domwarp3,
    billow,
    ridge,
};

vao nm_vao[NUM_NM] = {0};
PNC_Mesh nm_mesh[NUM_NM] = {0};

int main(int argc, char** argv) {
    int xres = 1920;
    int yres = 1080;

    int noise_mode = 0;

    gg_context *g = ggl_init("nxplore", xres, yres);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    uint32_t seed = 123456;

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
                        noise_mode = nm;
                        if (!nm_mesh[nm].tris) {
                            // if we need to generate this one
                            nm_mesh[nm] = generate_mesh(noise_funcs[nm], seed,
                                -5, 5, 1000,
                                -5, 5, 1000
                            );
                            nm_vao[nm] = ggl_upload_pnc(nm_mesh[nm]);
                        }
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
        glBindVertexArray(nm_vao[noise_mode]);
        glDrawArrays(GL_TRIANGLES, 0, nm_mesh[noise_mode].num_tris * 3); // handle should probably contain num triangles

        SDL_GL_SwapWindow(g->window);
    }

    ggl_teardown(g);
}

void frame(double dt) {

}