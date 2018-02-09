#version 400 core

layout (triangles, fractional_even_spacing, ccw) in;

in VertexData {
    vec3 color;
} tcs_in[];

out VertexData {
    vec3 color;
} tes_out;

void main(void){
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);

    tes_out.color = (gl_TessCoord.x * tcs_in[0].color) +
                    (gl_TessCoord.y * tcs_in[1].color) +
                    (gl_TessCoord.z * tcs_in[2].color);
}