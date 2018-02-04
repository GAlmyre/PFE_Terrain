#version 330 core

uniform vec3 v_color;

//in vec4 v_color;
in vec3 v_normal;
in vec2 v_texcoord;

out vec4 out_color;

void main(void) {
  out_color = vec4(v_color.xyz,1.0);
}
