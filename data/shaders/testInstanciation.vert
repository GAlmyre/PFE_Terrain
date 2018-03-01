#version 400 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;
uniform bool useModelMat;

uniform vec3 color;

in vec3 vtx_position;
in mat4 vtx_transformation;
in vec3 vtx_color;

out FragData {
  vec3 color;
} vs_to_fs;

void main()
{
  //vec3 pos = vec3(vtx_position.x+gl_InstanceID, vtx_position.yz);
  vec3 pos = vtx_position;
  if(useModelMat)
    gl_Position = projection*view*model*vec4(pos, 1.0);
  else
    gl_Position = projection*view*vtx_transformation*vec4(pos, 1.0);
  vs_to_fs.color = vtx_color;//vec3(0.9, 0.1, 0.1);
}
