#version 400 core

in FragData {
  vec3 color;
} fs_in;

out vec4 out_color;

void main(void) {
  out_color = vec4(fs_in.color, 1.);
}
