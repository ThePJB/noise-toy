#include "proc_tree.h"
#include "util.h"
#include "mymath.h"

tree_parameters tree_start(int trunk_sides, vec3s foliage_colour, vec3s trunk_colour) {
    const int array_initial_size = 5;
    tree_parameters tp = {
        .trunk_sides = trunk_sides,
        .foliage = malloc(sizeof(foliage) * array_initial_size),
        .trunk = malloc(sizeof(trunk_segment) * array_initial_size),
        .n_foliage = 0,
        .foliage_size = array_initial_size,
        .n_trunk_segments = 0,
        .trunk_segments_size = array_initial_size,
        .foliage_colour = foliage_colour,
        .trunk_colour = trunk_colour,
    };
    return tp;
}

void tree_push_trunk_segment(tree_parameters *tp, trunk_segment ts) {
    if (tp->n_trunk_segments >= tp->trunk_segments_size) {
        tp->trunk_segments_size *= 2;
        tp->trunk = realloc(tp->trunk, sizeof(trunk_segment) * tp->trunk_segments_size);
    }
    tp->trunk[tp->n_trunk_segments++] = ts;
}

void tree_push_foliage(tree_parameters *tp, foliage fp) {
    if (tp->n_foliage >= tp->foliage_size) {
        tp->foliage_size *= 2;
        tp->foliage = realloc(tp->foliage, sizeof(foliage) * tp->foliage_size);
    }
    tp->foliage[tp->n_foliage++] = fp;
}

// todo use tree start
// or remove it and use a separate thing that just acts on meshes
void tree_push_to_mesh(PNC_Mesh *m, tree_parameters tp, mat4s transform) {

    // Trunk
    // todo add cone case
    for (int i = 0; i < tp.n_trunk_segments; i++) {
        pnc_push_trunc_cone(m, tp.trunk_colour, 
            tp.trunk[i].top.position, tp.trunk[i].top.axis,
            tp.trunk[i].bot.position, tp.trunk[i].bot.axis,
            tp.trunk[i].top.radius, tp.trunk[i].bot.radius,
            tp.trunk_sides,
            transform
        );
    }


    // Foliage
    if (tp.foliage_type == FT_CONE) {
        for (int i = 0; i < tp.n_foliage; i++) {
            // todo noise_cone
            // todo phase
            /*
            pnc_push_cone(m, tp.foliage_colour,
                tp.foliage[i].pos_top, tp.foliage[i].pos_base, 
                glms_vec3_normalize(glms_vec3_sub(tp.foliage[i].pos_top, tp.foliage[i].pos_base)),
                tp.foliage[i].radius, tp.foliage[i].sides
            );
            */
        }
    } else {
        for (int i = 0; i < tp.n_foliage; i++) {
            vec3s center = glms_vec3_lerp(tp.foliage[i].pos_top, tp.foliage[i].pos_base, 0.5);
            vec3s axis = glms_vec3_sub(tp.foliage[i].pos_top, tp.foliage[i].pos_base);
            float d = tp.foliage[i].radius;
            float h = d;
            float circ_poly = tp.foliage[i].sides;


            pnc_push_ellipsoid(m, tp.foliage_colour, center, axis, d, h, circ_poly, circ_poly, transform);
        }
    }
    
}

void l_tree(int seed, tree_parameters *tp, trunk_cross_section tcs, l_tree_params ltp, float t) {
    const float trunk_branch_percentage_multiplier = 0;
    const float trunk_fatness_multiplier = 1.5;

    
    float distance = ltp.d_const + ltp.d_linear*t;

    if (t < 0) {
        tree_push_foliage(tp, (foliage) {
            .sides = 6,
            .radius = 0.4,
            .pos_top = (vec3s) {
                .x = tcs.position.x,
                .y = tcs.position.y + 0.2,
                .z = tcs.position.z,
            },
            .pos_base = (vec3s) {
                .x = tcs.position.x,
                .y = tcs.position.y - 0.2,
                .z = tcs.position.z,
            }
        });
        return;
    };

    //bool trunk = t / ltp.tree_t > ltp.trunk_percentage;

    float t_trunk_done = ltp.tree_t * (1 - ltp.trunk_percentage);
    float trunk_doneness = remap(ltp.tree_t, ltp.tree_t * (1 - ltp.trunk_percentage), 1, 0, t);
    
    bool trunk = trunk_doneness > 0;

    if (trunk) {
        vec3s new_axis = tcs.axis;
        new_axis.x += hash_floatn(seed+1234, -ltp.branch_range/4, ltp.branch_range/4);
        new_axis.z += hash_floatn(seed+5678, -ltp.branch_range/4, ltp.branch_range/4);
        new_axis = glms_vec3_normalize(new_axis);  
        new_axis = glms_vec3_lerp(new_axis, (vec3s){0,1,0}, ltp.up_tendency * 2);

        float new_radius = trunk_fatness_multiplier * ltp.thickness * trunk_doneness + (ltp.thickness * t_trunk_done);      
        vec3s new_pos = glms_vec3_add(tcs.position, glms_vec3_scale(new_axis, distance));
        trunk_cross_section new_cs = (trunk_cross_section) {
            .position = new_pos,
            .radius = new_radius,
            .axis = new_axis,
        };
        tree_push_trunk_segment(tp, (trunk_segment) {
            .bot = tcs,
            .top = new_cs
        });
        l_tree(hash(seed, 98766), tp, new_cs, ltp, t - ltp.t_increment);

        // branches?
        if (hash_floatn(seed+986482, 0, 1) < trunk_branch_percentage_multiplier * ltp.branch_keep_chance * (1 - trunk_doneness)) {
            float angle = hash_floatn(seed+320981234, 0, 2*M_PI);
            vec3s angle_vec = (vec3s) {cosf(angle), 0, sinf(angle)};
            vec3s new_axis = glms_vec3_cross(angle_vec, tcs.axis);
            new_axis = glms_vec3_normalize(new_axis);
            vec3s new_pos = glms_vec3_add(tcs.position, glms_vec3_scale(new_axis, distance));
            trunk_cross_section new_cs = (trunk_cross_section) {
                .axis = new_axis,
                .position = new_pos,
                .radius = t_trunk_done * ltp.thickness,
            };
            tree_push_trunk_segment(tp, (trunk_segment) {
                .bot = tcs,
                .top = new_cs,
            });
            
            l_tree(hash(seed, 23489234), tp, new_cs, ltp, t_trunk_done);
        }
        return;
    }



    // always bifurcate
    vec3s new_axis = tcs.axis;
    new_axis.x += hash_floatn(seed+1234, -ltp.branch_range, ltp.branch_range);
    new_axis.z += hash_floatn(seed+5678, -ltp.branch_range, ltp.branch_range);
    new_axis = glms_vec3_normalize(new_axis);
    vec3s opposing_new_axis = glms_vec3_rotate(new_axis, M_PI, tcs.axis);
    
    new_axis = glms_vec3_lerp(new_axis, (vec3s){0,1,0}, ltp.up_tendency);
    opposing_new_axis = glms_vec3_lerp(opposing_new_axis, (vec3s){0,1,0}, ltp.up_tendency);

    // still normalized i think?

    float new_radius = ltp.thickness * t;
    vec3s new_pos = glms_vec3_add(tcs.position, glms_vec3_scale(new_axis, distance));
    vec3s new_opp_pos = glms_vec3_add(tcs.position, glms_vec3_scale(opposing_new_axis, distance));

    trunk_cross_section new_cs = (trunk_cross_section) {
        .position = new_pos,
        .radius = new_radius,
        .axis = new_axis,
    };

    trunk_cross_section new_opp_cs = (trunk_cross_section) {
        .position = new_opp_pos,
        .radius = new_radius,
        .axis = opposing_new_axis,
    };

    tree_push_trunk_segment(tp, (trunk_segment) {
        .bot = tcs,
        .top = new_cs
    });



    l_tree(hash(seed, 98766), tp, new_cs, ltp, t - ltp.t_increment);
    if (hash_floatn(seed+986482, 0, 1) < ltp.branch_keep_chance) {
        tree_push_trunk_segment(tp, (trunk_segment) {
            .bot = tcs,
            .top = new_opp_cs
        });
        l_tree(hash(seed, 23489234), tp, new_opp_cs, ltp, t - ltp.t_increment);
    }
}