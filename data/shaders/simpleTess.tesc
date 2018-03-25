#version 400 core

layout(vertices = 3) out;

in VertexData {
    vec4 position;
    vec2 texcoord;
    float edgeLOD;
    float faceLOD;
} tcs_in[];

out VertexData {
    vec2 texcoord;
} tcs_out[];

const int CONSTANT = 0;
const int ADAPTATIVE_DISTANCE = 1;
const int ADAPTATIVE_VIEWSPACE = 2;
const int ADAPTATIVE_ELEVATION = 3;

uniform float TessLevelInner;
uniform vec3 TessLevelOuter;
uniform vec3 TessDistRefPos;
uniform float triEdgeSize;
uniform int tessMethod;
uniform float heightScale;
uniform vec2 viewport;
uniform sampler2D heightmap;
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform float adaptativeFactor;

#define ID gl_InvocationID

float distanceLOD(vec3 p0, vec3 p1) {
    float d = distance(p0, p1) + 0.0001; // Avoid divide by 0
    return min(100 * triEdgeSize / d, 64) * adaptativeFactor;
}

float viewspaceLOD(vec4 center, float radius) {
    vec4 sc0 = view * model * center;
    vec4 sc1 = sc0 - vec4(vec3(radius), 0.f);
    sc0 = sc0 + vec4(vec3(radius), 0.f);

    vec4 clip0 = projection * sc0;
    vec4 clip1 = projection * sc1;

    clip0 /= clip0.w;
    clip1 /= clip1.w;

    clip0.xy *= viewport;
    clip1.xy *= viewport;

    float d = distance(clip0, clip1);
    // d = 1 -> no tesselation -> return 1
    // d = 64 -> tesselation ->return 64

    return clamp(d / 30, 0, 64) * adaptativeFactor;
}

float projectedLOD(vec3 point, float elevation) {
    vec4 clip0 = projection * view * model * vec4(point, 1.f);
    vec4 clip1 = projection * view * model * vec4(point.x, point.y + elevation, point.z, 1.f);

    vec2 ndc0 = clip0.xy / clip0.w * viewport * 0.5;
    vec2 ndc1 = clip1.xy / clip1.w * viewport * 0.5;

    float dist = distance(ndc0, ndc1);

    return clamp(dist, 0.f, 64.f) * adaptativeFactor;
}

void main() {
    if (gl_InvocationID == 0) {
        if(tessMethod == CONSTANT) {
            gl_TessLevelInner[0] = TessLevelInner;
            gl_TessLevelOuter[0] = TessLevelOuter.x;
            gl_TessLevelOuter[1] = TessLevelOuter.y;
            gl_TessLevelOuter[2] = TessLevelOuter.z;
        } else if (tessMethod == ADAPTATIVE_DISTANCE) {
            vec3 v0 = tcs_in[0].position.xyz;
            vec3 v1 = tcs_in[1].position.xyz;
            vec3 v2 = tcs_in[2].position.xyz;

            vec3 centerEdge0 = v1 + (v2 - v1) / 2.f;
            vec3 centerEdge1 = v2 + (v0 - v2) / 2.f;
            vec3 centerEdge2 = v0 + (v1 - v0) / 2.f;

            float lvl0 = distanceLOD(centerEdge0, TessDistRefPos);
            float lvl1 = distanceLOD(centerEdge1, TessDistRefPos);
            float lvl2 = distanceLOD(centerEdge2, TessDistRefPos);

            gl_TessLevelInner[0] = 1.f / 3.f * (lvl0 + lvl1 + lvl2);
            gl_TessLevelOuter[0] = lvl0;
            gl_TessLevelOuter[1] = lvl1;
            gl_TessLevelOuter[2] = lvl2;
        } else if (tessMethod == ADAPTATIVE_VIEWSPACE) {
            vec4 v0 = tcs_in[0].position;
            vec4 v1 = tcs_in[1].position;
            vec4 v2 = tcs_in[2].position;

            v0.y = texture(heightmap, tcs_in[0].texcoord).r * heightScale;
            v1.y = texture(heightmap, tcs_in[1].texcoord).r * heightScale;
            v2.y = texture(heightmap, tcs_in[2].texcoord).r * heightScale;

            vec4 centerEdge0 = v1 + (v2 - v1) / 2.f;
            vec4 centerEdge1 = v2 + (v0 - v2) / 2.f;
            vec4 centerEdge2 = v0 + (v1 - v0) / 2.f;

            float radius0 = distance(v1, v2) * 0.5f;
            float radius1 = distance(v0, v2) * 0.5f;
            float radius2 = distance(v0, v1) * 0.5f;

            float lvl0 = viewspaceLOD(centerEdge0, radius0);
            float lvl1 = viewspaceLOD(centerEdge1, radius1);
            float lvl2 = viewspaceLOD(centerEdge2, radius2);

            gl_TessLevelInner[0] = 1.f / 3.f * (lvl0 + lvl1 + lvl2);
            gl_TessLevelOuter[0] = lvl0;
            gl_TessLevelOuter[1] = lvl1;
            gl_TessLevelOuter[2] = lvl2;
        } else if (tessMethod == ADAPTATIVE_ELEVATION) {
            vec3 v0 = tcs_in[0].position.xyz;
            vec3 v1 = tcs_in[1].position.xyz;
            vec3 v2 = tcs_in[2].position.xyz;

            vec3 centerEdge0 = v1 + (v2 - v1) / 2.f;
            vec3 centerEdge1 = v2 + (v0 - v2) / 2.f;
            vec3 centerEdge2 = v0 + (v1 - v0) / 2.f;

            // Triangle Barycenter
            vec3 center = 2.f / 3.f * (centerEdge0 - v0) + v0;

            gl_TessLevelInner[0] = projectedLOD(center,      tcs_in[0].faceLOD * heightScale);
            gl_TessLevelOuter[0] = projectedLOD(centerEdge0, tcs_in[0].edgeLOD * heightScale);
            gl_TessLevelOuter[1] = projectedLOD(centerEdge1, tcs_in[1].edgeLOD * heightScale);
            gl_TessLevelOuter[2] = projectedLOD(centerEdge2, tcs_in[2].edgeLOD * heightScale);
        }
    }

    gl_out[gl_InvocationID].gl_Position = tcs_in[gl_InvocationID].position;
    tcs_out[gl_InvocationID].texcoord = tcs_in[gl_InvocationID].texcoord;
}
