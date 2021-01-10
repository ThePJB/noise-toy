#pragma once

#include <stdint.h>

char *slurp(const char *path);
#define len(X) (sizeof(X) / sizeof(X[0]))
uint64_t get_us(); 

uint32_t hash(int position, uint32_t seed);
uint32_t hash2(int posx, int posy, uint32_t seed);
float hash_floatn(int position, float min, float max);