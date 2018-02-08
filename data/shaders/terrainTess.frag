#version 400 core

in FragData {
    vec2 texcoord;
    vec3 worldPos;
} fs_in;

out vec4 FragColor;

void main() {
    FragColor = vec4(1.0f, 1.0f, 1.0f, 1.0f);
//    FragColor = vec4(fs_in.texcoord, 0.0f, 1.0f);
}