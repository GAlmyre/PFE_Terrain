#version 400 core

in vec3 v_color;
out vec4 out_color;


void main(void) {
  out_color = vec4(1, 0, 0, 1.);
  out_color = vec4(v_color, 1.);
}
