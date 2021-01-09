#pragma once

#include "ggl.h"
#include <cglm/struct.h>


typedef struct {
    unsigned int texture;
    int size_x;
    int size_y;
    int bearing_x;
    int bearing_y;
    unsigned int advance;
} ft_char;

typedef struct {
    float scale;
    vec3s colour;
} text_style;

void text_init(gg_context *g);
void draw_text(const char* text, float x, float y, text_style style);
