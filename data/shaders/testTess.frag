#version 400 core

in TES_OUT {
    vec3 color;
} fs_in;

out vec4 FragColor;

void main() {
    FragColor = vec4(fs_in.color, 1.0f);
}