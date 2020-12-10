#include <SDL.h>
#include <stdbool.h>
#include "gef.h"

#define U32_MAX 0xFFFFFFFF

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

float fastfloor(float x) {
    if (x > 0) {
        return (int)x;
    } else if (x < 0) {
        return (int)(x-1);
    } else {
        return 0;
    }
}

float fast_abs(float x) {
    return x > 0 ? x : -x;
}

float frac(float x) {
    return x - fastfloor(x);
}

float lerp(float a, float b, float t) {
    return t*b + (1-t)*a;
}

float bilinear(float a, float b, float c, float d, float t1, float t2) {
    return lerp(lerp(a,b,t1),lerp(c,d,t1),t2);
}

float quant_err(float x, float y, uint32_t seed) {
    return (x - fastfloor(x) + y - fastfloor(y)) / 2;
}

float noise2_bilinear(float x, float y, uint32_t seed) {
    // actually need to sub kx/2 or something
    // kernel will be uniform rn anyway for this scale

    return bilinear(
        (double) squirrel3_2(x, y, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y, seed) / U32_MAX,
        (double) squirrel3_2(x, y + 1, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y + 1, seed) / U32_MAX,
        frac(x),
        frac(y)
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

int main(int argc, char** argv) {
    gef_init();
    gef_set_res(1024, 720);
    gef_set_name("nxplore");

    float (*noise_func)(float x, float y, uint32_t seed) = quant_err;
    uint32_t seed = 123456;

    transform_2d t = {
        0, 0, 10.0, 10.0 * 3.0 / 4.0
    };

    bool keep_going = true;
    while(keep_going) {
        // Handle Input
        SDL_Event e;
        while (SDL_PollEvent(&e) != 0) {
            if (e.type == SDL_QUIT) keep_going = false;
            if (e.type == SDL_KEYDOWN) {
                switch (e.key.keysym.sym) {
                    case SDLK_ESCAPE:
                        keep_going = false;
                        break;
                    case SDLK_1:
                        noise_func = quant_err;
                        break;
                    case SDLK_2:
                        noise_func = noise2_bilinear;
                        break;
                    case SDLK_3:
                        noise_func = fbm2_bilinear;
                        break;
                    case SDLK_4:
                        noise_func = fbm2_bilinear_domwarp1;
                        break;
                    case SDLK_5:
                        noise_func = fbm2_bilinear_domwarp2;
                        break;
                    case SDLK_6:
                        noise_func = fbm2_bilinear_domwarp3;
                        break;
                    case SDLK_7:
                        noise_func = billow;
                        break;
                    case SDLK_8:
                        noise_func = ridge;
                        break;

                    case SDLK_r:
                        seed = squirrel3(seed, 1);
                        printf("Reseed %u\n", seed);
                        break;

                }
            }
        }

        gef_clear();

        draw_noise(noise_func, t, seed);
        //draw_noise(fbm2_bilinear_domwarp1, t, 123456);
        //draw_noise(fbm2_bilinear, t, 123456);
        //draw_noise(noise2, t, 123456);
        //draw_noise(quant_err, t, 123456);

        gef_present();
    }

    gef_teardown();
}