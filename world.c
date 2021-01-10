#include "world.h"

PNC_Vert make_heightmap_vert(vec3s pos, vec3s normal) {

    const vec3s rock_colour = {0.6, 0.5, 0.4};
    
    const vec3s grass_low = {0, 1, 0};
    const vec3s grass_high = {1, 1, 0};

    const float grass_cutoff = 0.8;
    float upness = fabs(glms_vec3_dot(normal, (vec3s) {0, 1, 0}));

    vec3s colour;

    //printf("%.2f %.2f %.2f\n", normal.x, normal.y, normal.z);

    if (upness < grass_cutoff) {
        colour = rock_colour;
    } else {
        colour = glms_vec3_lerp(grass_low, grass_high, pos.y / 10);
    }

    return (PNC_Vert) {
            .pos = pos,
            .normal = normal, // todo maybe normalize for the scale
            .colour = colour,
    };
}

// x and y in 
void push_tree(PNC_Mesh *m, noise_transformer nt, noise2d_func f, float x, float z, l_tree_params ltp, uint32_t world_seed) {
    uint32_t tree_seed = hash2(999999*x, 999999*z, world_seed);


    pos_normal noise_at_tree = noise_tformer_sample(nt, f, x, z, world_seed);
    const vec3s green = (vec3s) {0.2, 0.6, 0.1};
    const vec3s brown = (vec3s) {0.4, 0.4, 0.1};
    const float rot_mag = 0.5;

    mat4s tree_trans = (mat4s) {
        1, 0, 0, 0,
        0, 1, 0, 0,
        0, 0, 1, 0,
        noise_at_tree.pos.x, noise_at_tree.pos.y, noise_at_tree.pos.z, 1,
    };

    mat4s tree_scale = (mat4s) {
        0.5, 0, 0, 0,
        0, 0.5, 0, 0,
        0, 0, 0.5, 0,
        0, 0, 0, 1,
    };


    tree_parameters tp = tree_start(4, green, brown);
    tp.foliage_type = FT_ELLIPSOID;
    trunk_cross_section base = (trunk_cross_section) {
        .position = noise_at_tree.pos,
        .axis = noise_at_tree.normal,
        .radius = hash_floatn(tree_seed+9871234, 0.1, 0.5),
    };

    mat4s tree_transform = glms_mat4_mul(tree_scale, tree_trans);

    l_tree(tree_seed, &tp, base, ltp, ltp.tree_t);
    tree_push_to_mesh(m, tp, tree_transform);
}

void push_terrain(PNC_Mesh *m, noise2d_func noise_func, uint32_t seed, int xsamples, int zsamples,
        noise_transformer nt,
        mat4s transform) {

    float stepx = 1.0 / xsamples;
    float stepz = 1.0 / zsamples;
    
    for (int i = 0; i < xsamples; i++) {
        for (int j = 0; j < zsamples; j++) {

            float x0 = i*stepx;
            float x1 = (i+1)*stepx;
            float z0 = j*stepz;
            float z1 = (j+1)*stepz;

            pos_normal h00 = noise_tformer_sample(nt, noise_func, x0, z0, seed);
            pos_normal h01 = noise_tformer_sample(nt, noise_func, x0, z1, seed);
            pos_normal h10 = noise_tformer_sample(nt, noise_func, x1, z0, seed);
            pos_normal h11 = noise_tformer_sample(nt, noise_func, x1, z1, seed);
            
            PNC_Vert v1, v2, v3;

            v1 = make_heightmap_vert(h00.pos, h00.normal);
            v2 = make_heightmap_vert(h10.pos, h10.normal);
            v3 = make_heightmap_vert(h01.pos, h01.normal);
            
            pnc_push_tri(m, v1, v2, v3, transform);
            
            v1 = make_heightmap_vert(h11.pos, h11.normal);
            
            pnc_push_tri(m, v2, v1, v3, transform);
        }
    }
}