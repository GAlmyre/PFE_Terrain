#version 400 core

layout(vertices = 3) out;

//in Vertex {
//    vec3 position;
//    vec3 color;
//} In[];
//
//out Vertex {
//    vec3 position;
//    vec3 color;
//} Out[];

#define ID gl_InvocationID

void main() {
    if (gl_InvocationID == 0) {
        gl_TessLevelInner[0] = 0.0;
        gl_TessLevelOuter[0] = 0.0;
        gl_TessLevelOuter[1] = 0.0;
        gl_TessLevelOuter[2] = 0.0;
    }
    gl_out[gl_InvocationID].gl_Position = gl_in[gl_InvocationID].gl_Position;
//    Out[ID].position = In[ID].position;
//    Out[ID].color = In[ID].color;
}
