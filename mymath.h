#pragma once

#include <cglm/struct.h>

float fastfloor(float x);
float fast_abs(float x);
float frac(float x);
float lerp(float a, float b, float t);
float interp3(float a, float b, float t);
float interp5(float a, float b, float t);
float bilinear(float a, float b, float c, float d, float t1, float t2);
float trilinear(float a, float b, float c, float d, float e, float f, float g, float h, float x, float y, float z);
float bilinear3(float a, float b, float c, float d, float t1, float t2);
float trilinear3(float a, float b, float c, float d, float e, float f, float g, float h, float x, float y, float z);
float quant_err(float x, float y, uint32_t seed);

#define min(A,B) A < B ? A : B
#define max(A,B) A > B ? A : B

vec3s normal_from_verts(vec3s a, vec3s b, vec3s c);

vec3s bilinear_normal(float a, float b, float c, float d, float t1, float t2);