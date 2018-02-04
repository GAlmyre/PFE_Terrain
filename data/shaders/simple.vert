#version 330 core

uniform mat4 proj_mat;
uniform mat4 view_mat;
uniform mat4 world_mat;

in vec3 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;
//in vec4 vtx_color;

//out vec4 v_color;
out vec3 v_normal;
out vec2 v_texcoord;

void main()
{
  gl_Position = proj_mat*view_mat*world_mat*vec4(vtx_position.x, vtx_position.y, vtx_position.z, 1.0);
  v_normal = vtx_normal;//TODO add normal matrix transformation ?
  v_texcoord = vtx_texcoord;
}
