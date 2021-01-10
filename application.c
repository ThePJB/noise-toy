#include "application.h"
#include "glad.h"
#include "util.h"

noise2d_func noise_funcs[NUM_NM] = {
    fbm2_domwarp3_fdm,
};

char *nm_name[NUM_NM] = {
    "alpha noise",
};

void switch_terrain(application *app, noise_mode nm) {
    app->current_noise_mode = nm;
    app->current_noise_func = noise_funcs[nm];
    printf("switched to %s\n", nm_name[nm]);
    if (!app->terrain[nm].tris) {
        // if we need to generate this one

        const float nsamples = 500;

        mat4s world_tform = glms_mat4_identity();

        //world_tform = glms_mat4_scale(world_tform, 10);

        l_tree_params ltp = (l_tree_params) {
            .tree_t = 2.0,
            .branch_keep_chance = 0.5,
            .d_const = 0.4,
            .d_linear = 0.1,
            .up_tendency = 0.1,
            .branch_range = 0.6,
            .t_increment = 0.15,
            .thickness = 0.2,
            .trunk_percentage = 0.6,
        };

        app->terrain[nm] = pnc_new(glms_mat4_identity());
        push_terrain(&app->terrain[nm], noise_funcs[nm], app->current_seed, nsamples, nsamples, app->tnt, world_tform);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.5, 0.5, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.25, 0.75, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.75, 0.25, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.75, 0.75, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.5, 0.75, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.75, 0.75, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.5, 0.25, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.25, 0.5, ltp, app->current_seed);
        push_tree(&app->terrain[nm], app->tnt, app->current_noise_func, 0.25, 0.25, ltp, app->current_seed);

        pnc_upload(&app->terrain[nm]);
    }
}

void handle_input(application *app, double dt) {
    // Events
    SDL_Event e;
    while (SDL_PollEvent(&e) != 0) {
        if (e.type == SDL_QUIT) {
            app->keep_going = false;
            return;
        }
        if (e.type == SDL_KEYDOWN) {
            SDL_Keycode sym = e.key.keysym.sym;
            if (sym == SDLK_ESCAPE) {
                app->keep_going = false;
                return;
            } else if (sym >= SDLK_1 && sym <= SDLK_9) {
                int nm = sym - SDLK_1;
                if (nm < NUM_NM) {
                    switch_terrain(app, nm);                       
                }
            }
        } else if (e.type == SDL_MOUSEMOTION) {
            app->cam = camera_update_look(app->cam, e.motion.xrel, e.motion.yrel);
        }
    }

    // Held
    const uint8_t *state = SDL_GetKeyboardState(NULL);
    app->cam = camera_update_move(app->cam, 10 * dt, 
        state[SDL_SCANCODE_W],
        state[SDL_SCANCODE_S],
        state[SDL_SCANCODE_A],
        state[SDL_SCANCODE_D],
        state[SDL_SCANCODE_SPACE],
        state[SDL_SCANCODE_LSHIFT]);
}

void draw(application *app) {

    // draw
    mat4s view = glms_lookat(
        app->cam.pos, 
        glms_vec3_add(app->cam.pos, app->cam.front),
        app->cam.up
    );

    mat4s proj = glms_perspective(
        glm_rad(app->cam.fovx), 
        (float)app->g->xres / (float)app->g->yres, 0.1, 10000
    );

    glClearColor(0.3, 0.5, 0.7, 1);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    pnc_draw(app->terrain[app->current_noise_mode], app->heightmap_pgm, view.raw[0], proj.raw[0]);

    SDL_GL_SwapWindow(app->g->window);
}

application application_init(int xres, int yres) {
    gg_context *g = ggl_init("nxplore", xres, yres);

    // make shaders
    char *vert = slurp("shaders/heightmap.vert");
    shader_id vert_id = ggl_make_shader(g, vert, GL_VERTEX_SHADER);
    char *frag = slurp("shaders/heightmap.frag");
    shader_id frag_id = ggl_make_shader(g, frag, GL_FRAGMENT_SHADER);
    shader_pgm_id heightmap_pgm = ggl_make_shader_pgm(g, vert_id, frag_id);
    free(vert);
    free(frag);

    mat3s terrain_input_tform = (mat3s) {
        1, 0, 0,
        0, 1, 0,
        0, 0, 1,
    };

    mat4s terrain_output_tform = (mat4s) {
        20, 0, 0, 0,
        0, 40, 0, 0,
        0, 0, 20, 0,
        0, 0, 0, 1,
    };

    noise_transformer tnt = noise_tformer_new(terrain_input_tform, terrain_output_tform);

    application app = (application) {
        .g = g,
        .cam = fly_camera(),
        .current_seed = 123456,
        .current_noise_mode = 0,
        .terrain = {0},
        .heightmap_pgm = heightmap_pgm,
        .keep_going = true,
        .tnt = tnt,
    };

    switch_terrain(&app, 0);

    return app;
}