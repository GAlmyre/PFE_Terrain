#version 400 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

in vec3 vtx_position;
in vec3 vtx_normal;
in vec2 vtx_texcoord;


//*
out FragData {
  vec2 texcoord;
} vs_to_fs;
//*/
//*
out VertexData {
  vec4 position;
  vec2 texcoord;
} vs_out;
//*/

void main()
{
  gl_Position = projection*view*model*vec4(vtx_position, 1.0);
  //gl_Position = vec4(vtx_position, 1.0);
  //vs_out.v_normal = vtx_normal;//TODO add normal matrix transformation ?
  vs_out.position = vec4(vtx_position, 1.);
  vs_out.texcoord = vtx_texcoord;
  vs_to_fs.texcoord = vtx_texcoord;
}
