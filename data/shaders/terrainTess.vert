#version 400 core

in vec3 vtx_position;
in vec2 vtx_texcoord;

out VertexData {
    vec2 texcoord;
} vs_out;

void main()
{
    gl_Position = vec4(vtx_position, 1.0);
    vs_out.texcoord = vtx_texcoord;
}