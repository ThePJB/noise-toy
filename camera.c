#include "camera.h"
#include "mymath.h"
#include <math.h>

camera fly_camera() {
    camera cam;

    cam.pos = (vec3s) {0,0,0};
    cam.front = (vec3s) {0,0,-1};
    cam.up = (vec3s) {0,1,0};
    cam.pitch = 0;
    cam.yaw = -90;
    cam.fovx = 90;

    return cam;
}

camera camera_update(camera c, int x, int y) {
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