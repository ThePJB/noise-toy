#pragma once

#include <cglm/struct.h>

typedef struct {
    vec3s pos;
    vec3s front;
    vec3s up;
    float pitch;
    float yaw;
    float fovx;
} camera;

camera fly_camera();
camera camera_update_look (camera c, int x, int y);
camera camera_update_move(camera c, float distance, bool forward, bool backward, bool left, bool right, bool up, bool down);