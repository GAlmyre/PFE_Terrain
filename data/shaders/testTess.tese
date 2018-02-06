#version 400 core

layout (triangles, equal_spacing, cw) in;

in VS_OUT {
    vec3 color;
} tes_in[];

out TES_OUT {
    vec3 color;
} tes_out;

void main(void){
    gl_Position = (gl_TessCoord.x * gl_in[0].gl_Position) +
                  (gl_TessCoord.y * gl_in[1].gl_Position) +
                  (gl_TessCoord.z * gl_in[2].gl_Position);

    tes_out.color = (gl_TessCoord.x * tes_in[0].color) +
                    (gl_TessCoord.y * tes_in[1].color) +
                    (gl_TessCoord.z * tes_in[2].color);
}