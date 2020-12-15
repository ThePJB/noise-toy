#include <SDL.h>
#include <cglm/struct.h>
#include <stdbool.h>
#include <math.h>
#include "ggl.h"
#include "mymath.h"
#include "util.h"
#include "glad.h"

typedef struct {
    vec3s pos;
    vec3s front;
    vec3s up;
    float pitch;
    float yaw;
    float fovx;
} camera;

camera fly_camera() {
    camera cam;

    cam.pos = (vec3s) {0,0,0};
    cam.front = (vec3s) {0,0,-1};
    cam.up = (vec3s) {0,1,0};
    cam.pitch = 0;
    cam.yaw = -90;
    cam.fovx = 90;

    return cam;
}

#define U32_MAX 0xFFFFFFFF
#define len(X) (sizeof(X) / sizeof(X[0]))

uint32_t squirrel3(int position, uint32_t seed) {
    const unsigned int BIT_NOISE1 = 0xB5297A4D;
    const unsigned int BIT_NOISE2 = 0x68E31DA4;
    const unsigned int BIT_NOISE3 = 0x1B56C4E9;

    unsigned int mangled = position;
    mangled *= BIT_NOISE1;
    mangled += seed;
    mangled ^= (mangled >> 8);
    mangled += BIT_NOISE2;
    mangled ^= (mangled << 8);
    mangled *= BIT_NOISE3;
    mangled ^= (mangled >> 8);
    return mangled;
}

uint32_t squirrel3_2(int posx, int posy, uint32_t seed) {
    const int PRIME_NUMBER = 198491317;
    return squirrel3(posx + posy * PRIME_NUMBER, seed);
}

uint32_t squirrel3_3(int posx, int posy, int posz, uint32_t seed) {
    const int PRIME1 = 198491317;
    const int PRIME2 = 6542989;

    return squirrel3(posx + posy * PRIME1 + PRIME2 * posz, seed);
}



// todo smooth it out (kernel interp etc)
// then do gradient noise, see how fast it can get and if it look good


float quant_err(float x, float y, uint32_t seed) {
    return (x - fastfloor(x) + y - fastfloor(y)) / 2;
}

// bug
float quant_err3(float x, float y, float z, uint32_t seed) {
    return frac(x) + frac(y) + frac(z) / 3;
}

float noise2_bilinear(float x, float y, uint32_t seed) {
    // actually need to sub kx/2 or something
    // kernel will be uniform rn anyway for this scale

    //return bilinear(
    return bilinear3(
        (double) squirrel3_2(x, y, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y, seed) / U32_MAX,
        (double) squirrel3_2(x, y + 1, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y + 1, seed) / U32_MAX,
        frac(x),
        frac(y)
        );
}

float noise3_trilinear(float x, float y, float z, uint32_t seed) {
    //return trilinear(
    return trilinear3(
        (double) squirrel3_3(x,y,z,seed) / U32_MAX,
        (double) squirrel3_3(x+1,y,z,seed) / U32_MAX,
        (double) squirrel3_3(x,y+1,z,seed) / U32_MAX,
        (double) squirrel3_3(x+1,y+1,z,seed) / U32_MAX,
        (double) squirrel3_3(x,y,z+1,seed) / U32_MAX,
        (double) squirrel3_3(x+1,y,z+1,seed) / U32_MAX,
        (double) squirrel3_3(x,y+1,z+1,seed) / U32_MAX,
        (double) squirrel3_3(x+1,y+1,z+1,seed) / U32_MAX,
        frac(x),
        frac(y),
        frac(z)
    );
}

float fbm2_bilinear(float x, float y, uint32_t seed) {
    float freq = 1;
    float amplitude = 1;
    float acc = 0;
    for (int i = 0; i < 5; i++) {
        acc += amplitude * noise2_bilinear(x*freq, y*freq, (1234+i)*seed);
        freq *= 2;
        amplitude /= 2;
    }
    return acc;
}

float fbm3(float x, float y, float z, uint32_t seed) {
    float freq = 1;
    float amplitude = 1;
    float acc = 0;
    for (int i = 0; i < 4; i++) {
        acc += amplitude * noise3_trilinear(x*freq, y*freq, z*freq, (1234+i)*seed);
        freq *= 2;
        amplitude /= 2;
    }
    return acc;
}

float billow(float x, float y, uint32_t seed) {
    return fast_abs(fbm2_bilinear(x, y, seed) * 2 - 2);
}

float ridge(float x, float y, uint32_t seed) {
    return 2 - billow(x, y, seed);
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
            //.colour = high_colour,
            .colour = glms_vec3_lerp(low_colour, high_colour, y),
    };
}
/*
PNC_Mesh generate_test_cube() {
    PNC_Mesh m = {0};
    m.num_tris = 12;
    m.tris = calloc(num_tris, sizeof(PNC_Tri));



}
*/

camera camera_update(camera c, int x, int y) {
    const float sensitivity = 0.05;

    float xf = sensitivity * x;
    float yf = sensitivity * y;

    c.yaw += xf;
    c.pitch -= yf;

    c.pitch = min(c.pitch, 89);
    c.pitch = max(c.pitch, -89);

    vec3s direction;
    direction.x = cos(glm_rad(c.yaw)) * cos(glm_rad(c.pitch));
    direction.y = sin(glm_rad(c.pitch));
    direction.z = sin(glm_rad(c.yaw)) * cos(glm_rad(c.pitch));
    c.front = glms_normalize(direction);

    return c;
}

// alloc and make a mesh
PNC_Mesh generate_mesh(float (*noise_func)(float x, float y, uint32_t seed),
        uint32_t seed,
        float startx, float endx, int xsamples,
        float starty, float endy, int ysamples) {

    //int num_quads = ((endx - startx) / stepx) *
                    //((endy - starty) / stepy);

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

int main(int argc, char** argv) {
    int xres = 1920;
    int yres = 1080;

    gg_context *g = ggl_init("nxplore", xres, yres);
    SDL_SetRelativeMouseMode(SDL_TRUE);

    float (*noise_func)(float x, float y, uint32_t seed) = quant_err;
    float (*noise_func3)(float x, float y, float z, uint32_t seed) = quant_err3;
    uint32_t seed = 123456;

    float t = 0;

    transform_2d tform = {
        0, 0, 10.0, 10.0 * 3.0 / 4.0
    };

    float (*noise_funcs[])(float x, float y, uint32_t seed) = {
        quant_err,
        noise2_bilinear,
        fbm2_bilinear,
        fbm2_bilinear_domwarp1,
        fbm2_bilinear_domwarp2,
        fbm2_bilinear_domwarp3,
        billow,
        ridge,
    };

    float  (*noise_funcs3[])(float x, float y, float z, uint32_t seed) = {
        quant_err3,
        noise3_trilinear,
        fbm3,
        fbm3_domwarp1,
    };

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

    // make world
    PNC_Mesh m = generate_mesh(fbm2_bilinear_domwarp3, seed,
        -5, 5, 1000,
        -5, 5, 1000
    );

    /*
    // make world
    PNC_Mesh m = generate_mesh(fbm2_bilinear_domwarp3, seed,
        -1, 0.5, 1,
        -1, 0.5, 1
    );
    */

    vao heightmap_vao = ggl_upload_pnc(m);
    free(m.tris);

    // camera

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
                    if (sym - SDLK_1 < len(noise_funcs)) {
                        noise_func = noise_funcs[sym -SDLK_1];
                    }
                    if (sym - SDLK_1 < len(noise_funcs3)) {
                        noise_func3 = noise_funcs3[sym - SDLK_1];
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
        glBindVertexArray(heightmap_vao);
        glDrawArrays(GL_TRIANGLES, 0, m.num_tris * 3); // handle should probably contain num triangles

        SDL_GL_SwapWindow(g->window);

        //printf("cam pos %.2f %.2f %.2f\n", cam.pos.x, cam.pos.y, cam.pos.z);
    }

/*
        gef_clear();

        if (do_2d) {
            draw_noise(noise_func, tform, seed);
        } else {
            draw_3d_noise_time(noise_func3, tform, seed, t);
            t += 0.03;
        }
        
        gef_present();
    }

    gef_teardown();
    */
    ggl_teardown(g);
}

void frame(double dt) {

}