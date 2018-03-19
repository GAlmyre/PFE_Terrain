#version 400 core

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

layout (location = 0) in vec3 vtx_position;

void main() {
	gl_Position = projection * view * model * vec4(vtx_position, 1.0);
}
