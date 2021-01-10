#include <SDL.h>
#include <cglm/struct.h>
#include <stdbool.h>
#include <math.h>
#include <unistd.h>

#include "ggl.h"
#include "pnc.h"
#include "mymath.h"
#include "util.h"
#include "glad.h"
#include "noise.h"
#include "camera.h"

#include "alpha_noise.h"

#include "proc_tree.h"

#include "application.h"


int main(int argc, char** argv) {
    application app = application_init(1920, 1080);

    int fps = 60;
    int64_t frame_us = 1000000 / fps;

    SDL_SetRelativeMouseMode(SDL_TRUE);
    
    double dt = 0;
    while(app.keep_going) {
        int64_t tstart = get_us();
        handle_input(&app, dt);
        draw(&app);

        pos_normal rc = noise_tformer_sample_from_output(
            app.tnt, app.current_noise_func, 
            app.cam.pos.x, app.cam.pos.z, app.current_seed
        );

        printf("cam pos:\n");
        printf("%.3f %.3f %.3f\n", app.cam.pos.x, app.cam.pos.y, app.cam.pos.z);
        printf("terrain pos at pos:\n");
        printf("%.3f %.3f %.3f\n", rc.pos.x, rc.pos.y, rc.pos.z);
        printf("terrain normal at pos:\n");
        printf("%.3f %.3f %.3f\n", rc.normal.x, rc.normal.y, rc.normal.z);

        int64_t tend = get_us();
        int64_t remaining_us = tend - tstart;
        dt = ((double)remaining_us) / 1000000;
        if (remaining_us < frame_us) {
            usleep(remaining_us);
        }
    }

    ggl_teardown(app.g);
}
