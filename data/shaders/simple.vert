#version 400 core

layout (location = 0) in vec3 vtx_position;
layout (location = 1) in vec2 vtx_texcoord;
layout (location = 2) in float vtx_edgeLOD;
layout (location = 3) in float vtx_faceLOD;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

//*
out FragData {
  vec3 viewDirection;
  vec2 texcoord;
  float tessLevel;
  float distance;
} vs_to_fs;
//*/
//*
out VertexData {
  vec4 position;
  vec2 texcoord;
  float edgeLOD;
  float faceLOD;
} vs_out;
//*/

void main()
{
  gl_Position = projection * view * model * vec4(vtx_position, 1.0);

  vs_out.position = vec4(vtx_position, 1.);
  vs_out.texcoord = vtx_texcoord;
  vs_out.edgeLOD = vtx_edgeLOD;
  vs_out.faceLOD = vtx_faceLOD;

  vs_to_fs.viewDirection = -gl_Position.xyz;
  vs_to_fs.texcoord = vtx_texcoord;
  vs_to_fs.tessLevel = vtx_faceLOD;
  vs_to_fs.distance = length((view*model*vec4(vtx_position,1.)).xyz);
}
