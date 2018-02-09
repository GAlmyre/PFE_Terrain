#version 400 core

layout(vertices = 3) out;

in VertexData {
    vec4 position;
    vec2 texcoord;
} tcs_in[];

out VertexData {
    vec2 texcoord;
} tcs_out[];

const int CONSTANT = 0;
const int APADTIVE_DISTANCE = 1;
const int APADTIVE_VIEWSPACE = 2;

uniform float TessLevelInner;
uniform vec3 TessLevelOuter;
uniform vec3 TessDistRefPos;
uniform float triEdgeSize;

#define ID gl_InvocationID

float distanceLOD(vec3 p0, vec3 p1) {
    float d = distance(p0, p1) + 0.0001; // Avoid divide by 0
    return min(100 * triEdgeSize / d, 64);
}

void main() {
    if (gl_InvocationID == 0) {
//        gl_TessLevelInner[0] = TessLevelInner;
//        gl_TessLevelOuter[0] = TessLevelOuter.x;
//        gl_TessLevelOuter[1] = TessLevelOuter.y;
//        gl_TessLevelOuter[2] = TessLevelOuter.z;
//
        vec3 v0 = tcs_in[0].position.xyz;
        vec3 v1 = tcs_in[1].position.xyz;
        vec3 v2 = tcs_in[2].position.xyz;

        vec3 middle0 = v1 + (v2 - v1) / 2.f;
        vec3 middle1 = v2 + (v0 - v2) / 2.f;
        vec3 middle2 = v0 + (v1 - v0) / 2.f;

        float lvl0 = distanceLOD(middle0, TessDistRefPos);
        float lvl1 = distanceLOD(middle1, TessDistRefPos);
        float lvl2 = distanceLOD(middle2, TessDistRefPos);

        gl_TessLevelInner[0] = 1.f / 3.f * (lvl0 + lvl1 + lvl2);
        gl_TessLevelOuter[0] = lvl0;
        gl_TessLevelOuter[1] = lvl1;
        gl_TessLevelOuter[2] = lvl2;
    }

    gl_out[gl_InvocationID].gl_Position = tcs_in[gl_InvocationID].position;
    tcs_out[gl_InvocationID].texcoord = tcs_in[gl_InvocationID].texcoord;
}
