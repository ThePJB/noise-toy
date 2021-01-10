#include "ggl.h"
#include "pnc.h"
#include "glad.h"
#include "mymath.h"

void pnc_upload(PNC_Mesh *m) {
    vbo *m_vbo = &(m->vbo);
    vao *m_vao = &(m->vao);
    glGenVertexArrays(1, m_vao);
    glGenBuffers(1, m_vbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, m->vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(PNC_Tri) * m->num_tris, m->tris, GL_STATIC_DRAW);
    
    size_t offset = 0;

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PNC_Vert), (void*)offset);
    glEnableVertexAttribArray(0);
    offset += 3 * sizeof(float);

    // true is for normalize (normal)
    // normal
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PNC_Vert), (void*)offset);
    glEnableVertexAttribArray(1);
    offset += 3 * sizeof(float);
    
    // colour
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, sizeof(PNC_Vert), (void*)offset);
    glEnableVertexAttribArray(2);
    offset += 3 * sizeof(float);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m->uploaded = true;
}

void pnc_draw(PNC_Mesh m, shader_pgm_id pgm, float *view, float *proj) {
    if (m.uploaded == false) {
        printf("warning: tried to draw mesh which hasn't been uploaded\n");
    }

    glUseProgram(pgm);
    glUniformMatrix4fv(glGetUniformLocation(pgm, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(pgm, "proj"), 1, GL_FALSE, proj);
    glUniformMatrix4fv(glGetUniformLocation(pgm, "model"), 1, GL_FALSE, m.model.raw[0]);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_TRIANGLES, 0, m.num_tris * 3); // handle should probably contain num triangles
}

// todo free from ram
// todo free from vram

PNC_Mesh pnc_new(mat4s model) {
    const size_t default_backing_mesh_size = 10;

    PNC_Mesh m = {0};
    m.tris = calloc(default_backing_mesh_size, sizeof(PNC_Tri));
    m.backing_length = default_backing_mesh_size;
    m.model = model;
    return m;
}

void pnc_push_tri(PNC_Mesh *m, PNC_Vert v1, PNC_Vert v2, PNC_Vert v3, mat4s transform) {
    if (m->num_tris >= m->backing_length) {
        m->tris = realloc(m->tris, m->backing_length * 2 * sizeof(PNC_Tri));
        m->backing_length *= 2;
    }

    v1.pos = glms_mat4_mulv3(transform, v1.pos, 1);
    v2.pos = glms_mat4_mulv3(transform, v2.pos, 1);
    v3.pos = glms_mat4_mulv3(transform, v3.pos, 1);

    // todo take care of the normals

    m->tris[m->num_tris][0] = v1;
    m->tris[m->num_tris][1] = v2;
    m->tris[m->num_tris][2] = v3;
    m->num_tris++;
}

PNC_Mesh pnc_test_mesh() {
    PNC_Mesh m = pnc_new(glms_mat4_identity());

    PNC_Vert v1, v2, v3;
    v1 = (PNC_Vert) {
        (vec3s) {-1, 0, 0},
        (vec3s) {0, 0, -1},
        (vec3s) {1, 0, 0}
    };
    v2 = (PNC_Vert) {
        (vec3s) {0, 1, 0},
        (vec3s) {0, 0, -1},
        (vec3s) {0, 1, 0}
    };
    v3 = (PNC_Vert) {
        (vec3s) {1, 0, 0},
        (vec3s) {0, 0, -1},
        (vec3s) {0, 0, 1}
    };
    pnc_push_tri(&m, v1, v2, v3, glms_mat4_identity());
    
    return m;
}

void pnc_push_trunc_cone(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s top_pos,
        vec3s top_axis,
        vec3s bot_pos,
        vec3s bot_axis,
        float r_top,
        float r_bot,
        int num_sides,
        mat4s transform) {

    float theta = 2 * M_PI / num_sides;

    vec3s utop = glms_vec3_cross(top_axis, glms_vec3_cross(top_axis, (vec3s) {1,0,0}));
    vec3s ubot = glms_vec3_cross(bot_axis, glms_vec3_cross(bot_axis, (vec3s) {1,0,0}));

    for (int i = 0; i < num_sides; i++) {
        vec3s top1 = glms_vec3_add(top_pos, glms_vec3_scale(glms_vec3_rotate(utop, i*theta, top_axis), r_top));
        vec3s top2 = glms_vec3_add(top_pos, glms_vec3_scale(glms_vec3_rotate(utop, (i+1)*theta, top_axis), r_top));
        
        vec3s bot1 = glms_vec3_add(bot_pos, glms_vec3_scale(glms_vec3_rotate(ubot, i*theta, bot_axis), r_bot));
        vec3s bot2 = glms_vec3_add(bot_pos, glms_vec3_scale(glms_vec3_rotate(ubot, (i+1)*theta, bot_axis), r_bot));

        vec3s normal1 = normal_from_verts(top1, top2, bot1);
        vec3s normal2 = normal_from_verts(top2, bot2, bot1); //should be identical

        pnc_push_tri(m, 
            (PNC_Vert) {top1, normal1, colour},
            (PNC_Vert) {top2, normal1, colour},
            (PNC_Vert) {bot1, normal1, colour},
            transform
        );

        pnc_push_tri(m, 
            (PNC_Vert) {top2, normal2, colour},
            (PNC_Vert) {bot2, normal2, colour},
            (PNC_Vert) {bot1, normal2, colour},
            transform
        );
    }
}

/*
void pnc_push_cone(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s top_pos,
        //vec3s top_axis,
        vec3s bot_pos,
        vec3s bot_axis,
        float r_bot,
        int num_sides,
        transform) {

    float theta = 2 * M_PI / num_sides;

    vec3s ubot = glms_vec3_cross(bot_axis, glms_vec3_cross(bot_axis, (vec3s) {1,0,0}));

    for (int i = 0; i < num_sides; i++) {
        vec3s bot1 = glms_vec3_add(bot_pos, glms_vec3_scale(glms_vec3_rotate(ubot, i*theta, bot_axis), r_bot));
        vec3s bot2 = glms_vec3_add(bot_pos, glms_vec3_scale(glms_vec3_rotate(ubot, (i+1)*theta, bot_axis), r_bot));

        vec3s normal1 = normal_from_verts(top_pos, bot1, bot2);

        pnc_push_tri(m, 
            (PNC_Vert) {top_pos, normal1, colour},
            (PNC_Vert) {bot1, normal1, colour},
            (PNC_Vert) {bot2, normal1, colour},
            transform
        );
    }
}
*/


void pnc_push_ellipsoid(
        PNC_Mesh *m,
        vec3s colour, 
        vec3s center,
        vec3s axis,
        float d,
        float h,
        int circ_sides,
        int h_sides,
        mat4s transform) {

        axis = glms_vec3_normalize(axis);
    
    for (int i = 0; i < h_sides; i++) {
        float theta_top = M_PI * ((float)i/h_sides);
        float theta_bot = M_PI * ((float)(i+1)/h_sides);
        
        float ert = ellipse_radius(h, d, theta_top);
        float erb = ellipse_radius(h, d, theta_bot);

        float r_top = sinf(theta_top) * ert;
        float r_bot = sinf(theta_bot) * erb;

        float dist_top = cosf(theta_top) * ert;
        float dist_bot = cosf(theta_bot) * erb;

        vec3s top_pos = glms_vec3_add(center, glms_vec3_scale(axis, dist_top));
        vec3s bot_pos = glms_vec3_add(center, glms_vec3_scale(axis, dist_bot));

        pnc_push_trunc_cone(m, colour, top_pos, axis, bot_pos, axis, r_top, r_bot, circ_sides, transform);
    }
}

void pnc_print_summary(PNC_Mesh m) {
    printf("Length: %zu\n", m.backing_length);
    printf("Num Tris: %zu\n", m.num_tris);
    printf("Tris ptr: %p\n", m.tris);
    printf("Vao: %u\n", m.vao);
    printf("Uploaded: %s\n", m.uploaded ? "true" : "false");    
}

void pnc_print_tris(PNC_Mesh m) {
    for (int i = 0; i < m.num_tris; i++) {
        printf("tri %d:\n", i);
        for (int j = 0; j < 3; j++) {
            PNC_Vert vert = m.tris[i][j];
            printf("\tvert %d:\n", j);
            printf("\t\tpos: %.2f %.2f %.2f\n", vert.pos.x, vert.pos.y, vert.pos.z);
            printf("\t\tnormal: %.2f %.2f %.2f\n", vert.normal.x, vert.normal.y, vert.normal.z);
            printf("\t\tcolour: %.2f %.2f %.2f\n", vert.colour.x, vert.colour.y, vert.colour.z);
        };
    }
}

void pnc_free_ram(PNC_Mesh m) {
    free(m.tris);
}

void pnc_free_vram(PNC_Mesh m) {
    glDeleteBuffers(1, &m.vbo);
    glDeleteVertexArrays(1, &m.vao);
}