#version 400 core

layout(vertices = 3) out;

in VS_OUT {
    vec3 color;
} tcs_in[];

out TCS_OUT {
    vec3 color;
} tcs_out[];

#define ID gl_InvocationID

void main() {
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 6.0;
        gl_TessLevelOuter[0] = 3.0;
        gl_TessLevelOuter[1] = 3.0;
        gl_TessLevelOuter[2] = 3.0;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].color = tcs_in[gl_InvocationID].color;
}
