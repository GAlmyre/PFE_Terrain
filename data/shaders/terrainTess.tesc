#version 400 core

layout(vertices = 3) out;

in VertexData {
    vec2 texcoord;
} tcs_in[];

out VertexData {
    vec2 texcoord;
} tcs_out[];

uniform float TessLevelInner;
uniform vec3 TessLevelOuter;

#define ID gl_InvocationID

void main() {
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = TessLevelInner;
        gl_TessLevelOuter[0] = TessLevelOuter.x;
        gl_TessLevelOuter[1] = TessLevelOuter.y;
        gl_TessLevelOuter[2] = TessLevelOuter.z;
    }

    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
    tcs_out[gl_InvocationID].texcoord = tcs_in[gl_InvocationID].texcoord;
}
