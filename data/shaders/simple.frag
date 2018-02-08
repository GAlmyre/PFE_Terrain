#version 400 core

uniform sampler2D heightmap;
uniform sampler2D texturemap;
uniform vec3 v_color;

uniform int texturing_mode;

uniform bool wireframe;

in FragData {
  vec2 texcoord;
} fs_in;

out vec4 out_color;

//if these values are changed  make sure to change them in TerrainScene the same way
const int TEXTURING_MODE_CONST_COLOR = 0;
const int TEXTURING_MODE_TEXTURE = 1;
const int TEXTURING_MODE_HEIGHTMAP = 2;
const int TEXTURING_MODE_NORMALS = 3;
const int TEXTURING_MODE_TEXCOORDS = 4;

void main(void) {
  out_color = vec4(v_color, 1.);
  if(!wireframe){
    switch(texturing_mode){
    case TEXTURING_MODE_CONST_COLOR:
      break;
    case TEXTURING_MODE_TEXTURE:
      out_color = vec4(texture(texturemap, fs_in.texcoord.xy).xyz, 1.);
      break;
    case TEXTURING_MODE_HEIGHTMAP:      
      out_color = vec4(texture(heightmap, fs_in.texcoord.xy).xyz, 1.);
      break;
    case TEXTURING_MODE_NORMALS:
      out_color = vec4(0.,1.,1.,1.);
      break;
    case TEXTURING_MODE_TEXCOORDS:
      out_color = vec4(fs_in.texcoord.x, 0., fs_in.texcoord.y, 1.);
      break;
    } 
  }
}
