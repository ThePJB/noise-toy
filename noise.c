#include <stdint.h>
#include "mymath.h"
/* 
    Hash functions 
    TODO - how cheap can you make them for terrain? like halve the instructions
        or something
*/
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

#define U32_MAX 0xFFFFFFFF


/* 
    base noise funcs
*/
float noise2_linear(float x, float y, uint32_t seed) {
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

float noise3_linear(float x, float y, float z, uint32_t seed) {
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

/* 
    fractal noise 
*/

float fbm2_bilinear4(float x, float y, uint32_t seed) {
    return (
        1.0000 * noise2_linear(x * 1, y * 1, seed + 1234) +
        0.5000 * noise2_linear(x * 2, y * 2, seed + 5466) +
        0.2500 * noise2_linear(x * 4, y * 4, seed + 9078) +
        0.1250 * noise2_linear(x * 8, y * 8, seed + 1704)
    ) / 1.875;
}

float fbm2_bilinear(float x, float y, uint32_t seed) {
    return (
        1.0000 * noise2_linear(x * 1, y * 1, seed + 1234) +
        0.5000 * noise2_linear(x * 2, y * 2, seed + 5466) +
        0.2500 * noise2_linear(x * 4, y * 4, seed + 9078) +
        0.1250 * noise2_linear(x * 8, y * 8, seed + 1704) +
        0.06125 * noise2_linear(x * 16, y * 16, seed + 4568)
    ) / 1.875;
}


float fbm3(float x, float y, float z, uint32_t seed) {
    return (
        1.0000 * noise3_linear(x * 1, y * 1, z * 1, seed + 1234) +
        0.5000 * noise3_linear(x * 2, y * 2, z * 2, seed + 5466) +
        0.2500 * noise3_linear(x * 4, y * 4, z * 4, seed + 9078) +
        0.1250 * noise3_linear(x * 8, y * 8, z * 8, seed + 1704)
    ) / 1.875;
}

float billow(float x, float y, uint32_t seed) {
    return (fast_abs(fbm2_bilinear(x, y, seed) - 0.5) * 2);
}

float ridge(float x, float y, uint32_t seed) {
    return 1 - billow(x, y, seed);
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