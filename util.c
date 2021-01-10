#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <time.h>

// todo make better
char *slurp(const char *path) {
    char *buffer = 0;
    long length;
    FILE * f = fopen (path, "rb");

    if (f) {
        fseek (f, 0, SEEK_END);
        length = ftell (f);
        fseek (f, 0, SEEK_SET);
        buffer = calloc(length + 1, 1);
        if (buffer) {
            (void)(fread(buffer, 1, length, f) + 1); // to make compiler shut up
        }
        fclose (f);
    }

    if (!buffer) {
        printf("problem loading file %s\n", path);
        exit(1);
    }
    return buffer;
}

uint64_t get_us() { 
    struct timespec t;
    clock_gettime(CLOCK_MONOTONIC, &t);
    return (uint64_t)t.tv_sec * 1000000 + (uint64_t)t.tv_nsec / 1000;
}

uint32_t hash(int position, uint32_t seed) {
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

uint32_t hash2(int posx, int posy, uint32_t seed) {
    const int PRIME_NUMBER = 198491317;
    return hash(posx + posy * PRIME_NUMBER, seed);
}

#define U32_MAX 0xFFFFFFFF

float hash_floatn(int position, float min, float max) {
    return ((double)hash(position,  2134123)/U32_MAX) * (max - min) + min;
}