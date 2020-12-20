#include "camera.h"
#include "mymath.h"
#include <math.h>


camera fly_camera() {
    camera c;

    c.pos = (vec3s) {0,0,0};
    c.front = (vec3s) {0,0,-1};
    c.up = (vec3s) {0,1,0};
    c.pitch = 0;
    c.yaw = -90;
    c.fovx = 90;

    return c;
}

camera camera_update_look(camera c, int x, int y) {
    const float sensitivity = 0.05;

    float xf = sensitivity * x;
    float yf = sensitivity * y;

    c.yaw += xf;
    c.pitch -= yf;

    c.pitch = min(c.pitch, 89);
    c.pitch = max(c.pitch, -89);

    vec3s direction;
    direction.x = cos(glm_rad(c.yaw)) * cos(glm_rad(c.pitch));
    direction.y = sin(glm_rad(c.pitch));
    direction.z = sin(glm_rad(c.yaw)) * cos(glm_rad(c.pitch));
    c.front = glms_normalize(direction);

    return c;
}

camera camera_update_move(camera c, float distance,
    bool forward,
    bool backward,
    bool left,
    bool right,
    bool up,
    bool down) {

    if (forward) {
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(c.front, distance));
    }
    if (backward) {
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(c.front, -1 * distance));
    }
    if (left) {
        vec3s cam_left = glms_normalize(glms_vec3_cross(c.up, c.front));
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(cam_left, distance));
    }
    if (right) {
        vec3s cam_left = glms_normalize(glms_vec3_cross(c.up, c.front));
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(cam_left, -1 * distance));
    }
    if (up) {
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(c.up, distance));
    }
    if (down) {
        c.pos = glms_vec3_add(c.pos, glms_vec3_scale(c.up, -1 * distance));
    }

    return c;
}