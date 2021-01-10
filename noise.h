#pragma once

uint32_t squirrel3(int position, uint32_t seed);
uint32_t squirrel3_2(int posx, int posy, uint32_t seed);
uint32_t squirrel3_3(int posx, int posy, int posz, uint32_t seed);
float noise2_bilinear(float x, float y, uint32_t seed);
float noise3_trilinear(float x, float y, float z, uint32_t seed);
float fbm2_bilinear(float x, float y, uint32_t seed);
float fbm3(float x, float y, float z, uint32_t seed);
float billow(float x, float y, uint32_t seed);
float ridge(float x, float y, uint32_t seed);
float fbm2_bilinear_domwarp1(float x, float y, uint32_t seed);
float fbm2_bilinear_domwarp2(float x, float y, uint32_t seed);
float fbm2_bilinear_domwarp3(float x, float y, uint32_t seed);
float fbm3_domwarp1(float x, float y, float z, uint32_t seed);

float billow_ridge(float x, float y, uint32_t seed);

typedef struct {
    float value;
    vec3s normal;
} noise_result;

typedef noise_result(*noise2d_func)(float, float, uint32_t);


noise_result noise2_normal(float x, float y, uint32_t seed);
noise_result fbm2_normal(float x, float y, uint32_t seed);