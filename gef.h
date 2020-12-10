#pragma once

#include <stdio.h>

#define gef_die(X) printf("%s %d %s: dying -- %s\n", __FILE__, __LINE__, __func__, X), gef_teardown()

void gef_init();
void gef_set_name(char *name);
void gef_set_res(int x, int y);
void gef_teardown();

int gef_get_xres();
int gef_get_yres();

void gef_put_pixel(int x, int y, int r, int g, int b, int a);
void gef_clear();
void gef_set_colour(int r, int g, int b, int a);
void gef_present();

