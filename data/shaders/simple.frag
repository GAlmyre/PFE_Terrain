#version 400 core

uniform sampler2D heightmap;
uniform sampler2D texturemap;
uniform vec3 v_color;

uniform int texturing_mode;

uniform float heightScale;

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

#define M_PI 3.1415926535897932384626433832795

vec3 normalFromTexcoords(vec2 uv, float elevation){
  vec2 normals = texture(heightmap, uv).yz;
  normals = normals*2. - 1.;
  vec3 u = normalize(vec3(1., elevation*normals.x, 0.));
  vec3 v = normalize(vec3(0., elevation*normals.y, 1.));
  return cross(v, u);
}

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
      float val = texture(heightmap, fs_in.texcoord.xy).x;
      out_color = vec4(val, val, val, 1.);
      break;
    case TEXTURING_MODE_NORMALS:
      vec3 n = normalFromTexcoords(fs_in.texcoord, heightScale/10. );
      n = (n+1)/2;
      out_color = vec4(n, 1.);
      break;
    case TEXTURING_MODE_TEXCOORDS:
      out_color = vec4(fs_in.texcoord.x, 0., fs_in.texcoord.y, 1.);
      break;
    } 
  }
}
