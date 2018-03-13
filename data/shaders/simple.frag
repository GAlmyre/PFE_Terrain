#version 400 core

uniform sampler2D heightmap;
uniform sampler2D texturemap;
uniform sampler2D colormap;
uniform vec3 v_color;

uniform int texturing_mode;

uniform float heightScale;

uniform bool wireframe;

uniform float Ka;
uniform float Kd;
uniform float Ks;
uniform float shininess;
uniform float distanceFog;
uniform vec3 fogColor;

uniform vec3 lightDirection;
uniform vec3 lightColor;

in FragData {
  vec3 viewDirection;
  vec2 texcoord;
  float tessLevel;
  float distance;
} fs_in;

out vec4 out_color;

//if these values are changed  make sure to change them in TerrainScene the same way
const int TEXTURING_MODE_CONST_COLOR = 0;
const int TEXTURING_MODE_TEXTURE = 1;
const int TEXTURING_MODE_HEIGHTMAP = 2;
const int TEXTURING_MODE_NORMALS = 3;
const int TEXTURING_MODE_TEXCOORDS = 4;
const int TEXTURING_MODE_TESSLEVEL = 5;

#define M_PI 3.1415926535897932384626433832795

vec3 normalFromTexcoords(vec2 uv, float elevation){
  vec2 normals = texture(heightmap, uv).yz;
  normals = (normals - 0.5) * 2.f;
  return normalize(vec3(elevation * normals.x, 2.f, elevation * normals.y));
}

vec3 shade(vec3 N, vec3 L, vec3 V,
           vec3 color, float Ka, float Kd, float Ks,
           vec3 lightCol, float shininess){

    vec3 final_color = color * Ka * lightCol;	//ambient

    float lambertTerm = dot(N,L);		//lambert

    if(lambertTerm > 0.0) {
        final_color += color * Kd * lambertTerm * lightCol; 	//diffuse

        vec3 R = reflect(-L,N);
        float specular = pow(max(dot(R,V), 0.0), shininess);
        final_color +=  Ks * lightCol * specular;	//specular
    }

    return final_color;
}

void main(void) {
  out_color = vec4(v_color, 1.);
  if(!wireframe){
    switch(texturing_mode){
    case TEXTURING_MODE_CONST_COLOR:
      break;
    case TEXTURING_MODE_TEXTURE:
      float Kaa = 0.4;
      float Kdd = 0.8;
      float Kss = 0.2;
      float sh = 50;
      vec3 lc = vec3(1,1,1);
      vec3 ld = normalize(vec3(1,1,1));

      vec3 normal = normalFromTexcoords(fs_in.texcoord, heightScale);
      vec3 diffuse = texture(texturemap, fs_in.texcoord.xy).xyz;
      vec3 color = shade(normal, normalize(lightDirection), normalize(fs_in.viewDirection), diffuse, Ka, Kd, Ks, lightColor, shininess);
      if(distanceFog > 0)
	color = mix(color, fogColor, min(1., fs_in.distance/distanceFog));
    out_color = vec4(color, 1);
      //out_color = vec4(vec3(fs_in.distance/600.,0,0), 1);
      /*
      float Ka = 0.4;
      float Kd = 0.8;
      float Ks = 0.2;
      float shininess = 50;
      vec3 lightColor = vec3(1,1,1);
      vec3 normal = normalFromTexcoords(fs_in.texcoord, heightScale);
      vec3 lightDir = normalize(vec3(1,1,1));
      vec3 diffuse = texture(texturemap, fs_in.texcoord.xy).xyz;
      vec3 color = shade(normalize(normal), lightDir, normalize(fs_in.viewDirection), diffuse, Ka, Kd, Ks, lightColor, shininess);
      out_color = vec4(color, 1); */
      break;
    case TEXTURING_MODE_HEIGHTMAP:
      float val = texture(heightmap, fs_in.texcoord.xy).x;
      out_color = vec4(val, val, val, 1.);
      break;
    case TEXTURING_MODE_NORMALS:
      vec3 n = normalFromTexcoords(fs_in.texcoord, heightScale);
      n = (n+1)/2;
      out_color = vec4(n, 1.);
      break;
    case TEXTURING_MODE_TEXCOORDS:
      out_color = vec4(fs_in.texcoord.x, 0., fs_in.texcoord.y, 1.);
      break;
    case TEXTURING_MODE_TESSLEVEL:
      out_color = texture(colormap, vec2(fs_in.tessLevel / 64., 0.));
      break;
    }
  }
}