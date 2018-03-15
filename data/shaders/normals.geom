#version 400 core

layout (triangles) in;
layout (line_strip, max_vertices = 6) out;

uniform sampler2D heightmap;
uniform float heightScale;
uniform mat4 projection;
uniform mat4 view;

in FragData {
    vec3 viewDirection;
    vec2 texcoord;
    float tessLevel;
    float distance;
} gs_in[];

const float MAGNITUDE = 30.f;

vec3 normalFromTexcoords(vec2 uv, float elevation){
  vec2 normals = texture(heightmap, uv).yz;
  normals = (normals - 0.5) * 2.f;
  return normalize(vec3(elevation * normals.x, 2.f, elevation * normals.y));
}

void generateLine(int index)
{
    vec4 normal = projection * view * vec4(normalFromTexcoords(gs_in[index].texcoord, heightScale)  * MAGNITUDE, 0.f);

    gl_Position = gl_in[index].gl_Position;
    EmitVertex();
    gl_Position = gl_in[index].gl_Position + normal;
    EmitVertex();
    EndPrimitive();
}

void main() {
    generateLine(0);
    generateLine(1);
    generateLine(2);
}
