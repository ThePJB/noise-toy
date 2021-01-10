#include <stdint.h>
#include "util.h"
#include "mymath.h"
#include "noise.h"

#define U32_MAX 0xFFFFFFFF

float noise2(float x, float y, uint32_t seed) {
    return bilinear3(
        (double) squirrel3_2(x, y, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y, seed) / U32_MAX,
        (double) squirrel3_2(x, y + 1, seed) / U32_MAX,
        (double) squirrel3_2(x + 1, y + 1, seed) / U32_MAX,
        frac(x),
        frac(y)
    );
}

float fbm2(float x, float y, uint32_t seed) {
    return (
        1.0000 * noise2(x * 1, y * 1, seed + 1234) +
        0.5000 * noise2(x * 2, y * 2, seed + 5466) +
        0.2500 * noise2(x * 4, y * 4, seed + 9078) +
        0.1250 * noise2(x * 8, y * 8, seed + 1704)
    ) / 1.875;
}

float fbm2_domwarp3(float x, float y, uint32_t seed) {
    //return fabs(x) * fabs(y);
    //return fbm2(x, y, seed);
    //return fbm2(x + fbm2(x, y, seed+745345), y + fbm2(x, y, seed+90809842), seed+345345);
    return fbm2(x + fbm2( x + fbm2(x, y, seed+1234), y + fbm2(x, y, seed+5678), seed+745345), y + fbm2(x + fbm2(x, y, seed+239873245), y + fbm2(x, y, seed+123489754), seed+90809842), seed+345345);
    //return fbm2_bilinear_domwarp1(x + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1337), y + warp_coeff * fbm2_bilinear_domwarp1(x, y, seed+1919), seed);

}

noise_result fbm2_domwarp3_fdm(float x, float z, uint32_t seed) {
    const float delta = 0.001;
    float height = fbm2_domwarp3(x, z, seed);
    float height_xo = fbm2_domwarp3(x - delta, z, seed);
    float height_zo = fbm2_domwarp3(x, z - delta, seed);

    // normal from x and y gradients, maybe a better way to do this
    // pen and paper

    float dydx = (height - height_xo) / delta;
    float dydz = (height - height_zo) / delta;

    vec3s normal = glms_vec3_cross(
        (vec3s) {0, dydz, 1},
        (vec3s) {1, dydx, 0}
    );

    normal = glms_vec3_normalize(normal);

    return (noise_result) {
        .value = height,
        .normal = normal,
    };
}