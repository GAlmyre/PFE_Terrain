#version 400 core

layout (triangles, equal_spacing, cw) in;

in VertexData {
    vec2 texcoord;
} tcs_in[];

out FragData {
    vec3 viewDirection;
    vec2 texcoord;
    float tessLevel;
    float distance;
} tes_out;

uniform float heightScale;
uniform sampler2D heightmap;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main(void){
    // Interpolate texcoords
    vec2 texcoord = (gl_TessCoord.x * tcs_in[0].texcoord) +
                    (gl_TessCoord.y * tcs_in[1].texcoord) +
                    (gl_TessCoord.z * tcs_in[2].texcoord);

    float displacement = texture(heightmap, texcoord).r * heightScale;
    // Interpolate position and apply displacement to y axis
    vec4 position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                    (gl_TessCoord.y * gl_in[1].gl_Position) +
                    (gl_TessCoord.z * gl_in[2].gl_Position) +
                    vec4(0.f, displacement, 0.f, 0.f);

    tes_out.distance = length((view * model * position).xyz);
    tes_out.texcoord = texcoord;
    gl_Position = projection * view * model * position;

    tes_out.viewDirection = -gl_Position.xyz;

    // compute distance from view


    // Pass tess levels
    tes_out.tessLevel = gl_TessLevelInner[0];
}
