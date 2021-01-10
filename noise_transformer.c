#include "noise_transformer.h"

noise_transformer noise_tformer_new(mat3s input, mat4s output) {
    mat4s normal_fixer = glms_mat4_transpose(glms_mat4_inv(output));
    return (noise_transformer) {
        .input = input,
        .output = output,
        .normal_fixer = normal_fixer,
        .output_inv = glms_mat4_inv(output),
    };
}

pos_normal noise_tformer_sample(noise_transformer nt, noise2d_func f, float x, float y, uint32_t seed) {
    vec3s input = (vec3s) {x, y, 1};
    input = glms_mat3_mulv(nt.input, input);

    noise_result result = f(input.x, input.y, seed);
    
    vec3s output_vec = (vec3s) {x, result.value, y};
    output_vec = glms_mat4_mulv3(nt.output, output_vec, 1);

    result.normal = glms_mat4_mulv3(nt.normal_fixer, result.normal, 1);
    result.normal = glms_vec3_normalize(result.normal);

    return (pos_normal) {
        output_vec,
        result.normal,
    };
}

pos_normal noise_tformer_sample_from_output(noise_transformer nt, noise2d_func f, float x, float y, uint32_t seed) {
    printf("sfo input %.2f %.2f\n", x, y);

    vec4s input = (vec4s) {x, 0, y, 1};
    input = glms_mat4_mulv(nt.output_inv, input);

    printf("sfo input tformed %.2f %.2f %.2f\n", input.x, input.y, input.z);

    noise_result result = f(input.x, input.z, seed);
    
    vec3s output_vec = (vec3s) {input.x, result.value, input.z};
    output_vec = glms_mat4_mulv3(nt.output, output_vec, 1);

    //result.normal = glms_mat4_mulv3(nt.normal_fixer, result.normal, 1);

    return (pos_normal) {
        output_vec,
        result.normal,
    };
}