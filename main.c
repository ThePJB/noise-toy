#include <SDL.h>
#include <stdbool.h>
#include "gef.h"
#include "mymath.h"

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

void draw_noise(float (*noise_func)(float x, float y, uint32_t seed), transform_2d t, uint32_t seed) {
    int xres = gef_get_xres();
    int yres = gef_get_yres();

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

int main(int argc, char** argv) {
    gef_init();
    //gef_set_res(1024, 720);
    gef_set_name("nxplore");

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

    bool do_2d = true;
    bool keep_going = true;
    while(keep_going) {
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
            }
        }

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
}