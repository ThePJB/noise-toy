#version 330 core

// there was some nice thing like namespacing or something

layout (location = 0) in vec3 pos;
layout (location = 1) in vec3 normal;
layout (location = 2) in vec3 colour;

out vec3 vert_colour;

uniform mat4 view;
uniform mat4 proj;

void main() {
    vec3 sun = vec3(0.1, 1, 0);
    gl_Position = proj * view * vec4(pos, 1);

    //vec3 vert_colour = gl_Position.xyz;
    //vert_colour = (normal + 1) / 2;
    float sunness = dot(normal, sun);
    vert_colour = colour * (0.7 + 0.3*sunness);
    //vert_colour = vec3(1,0,0);
}
