#version 400 core

uniform mat4 projection;
uniform mat4 view;
uniform vec3 vertex_position[1016];
in uint vtx_ID;

out vec3 v_color;

void main()
{
  if(vtx_ID == 0){
    gl_Position = projection*view*vec4(1.,0.,0.,1.);
    v_color = vec3(1, 0, 0);
  }
  if(vtx_ID == 1){
    gl_Position = projection*view*vec4(0.,1.,0.,1.);
    v_color = vec3(0, 1, 0);
  }
  if(vtx_ID == 2){
    gl_Position = projection*view*vec4(0.,0.,1.,1.);
    v_color = vec3(0,0,1);
  }
  v_color = vec3(0,1,0);
  gl_Position = projection*view*vec4(vertex_position[vtx_ID], 1.0);
}
