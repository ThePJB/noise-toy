#include "ggl.h"
#include "line.h"
#include "glad.h"

void line_upload(line_mesh *m) {
    vbo line_vbo;
    vao *m_vao = &(m->vao);
    
    glGenVertexArrays(1, m_vao);
    glGenBuffers(1, &line_vbo);
    glBindVertexArray(m->vao);
    glBindBuffer(GL_ARRAY_BUFFER, line_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(line_segment) * m->num_segs, m->lines, GL_STATIC_DRAW);
    
    size_t offset = 0;

    // pos
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(vec3s), (void*)0);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    m->uploaded = true;
}

void line_draw(line_mesh m, shader_pgm_id pgm, float *view, float *proj) {
    if (m.uploaded == false) {
        printf("warning: tried to draw mesh which hasn't been uploaded\n");
    }
    glEnable(GL_LINE_SMOOTH);
    glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    glLineWidth(1.0);

    glUseProgram(pgm);
    glUniformMatrix4fv(glGetUniformLocation(pgm, "view"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(pgm, "proj"), 1, GL_FALSE, proj);
    glBindVertexArray(m.vao);
    glDrawArrays(GL_LINES, 0, m.num_segs * 2);
}

line_mesh line_new() {
    const size_t default_backing_mesh_size = 10;

    line_mesh m = {0};
    m.lines = calloc(default_backing_mesh_size, sizeof(line_mesh));
    m.backing_length = default_backing_mesh_size;
    return m;    
}

void line_push(line_mesh *m, vec3s v1, vec3s v2) {
    if (m->num_segs >= m->backing_length) {
        m->lines = realloc(m->lines, m->backing_length * 2 * sizeof(line_segment));
        m->backing_length *= 2;
    }

    m->lines[m->num_segs++] = (line_segment) {
        .a = v1,
        .b = v2, 
    };
}

// todo add a mesh freeer and deleter