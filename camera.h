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
camera camera_update(camera c, int x, int y);