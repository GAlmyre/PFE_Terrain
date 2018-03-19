#version 430 core

layout (std430, binding=1) buffer vertexData
{
  float vertex_data[];
};

layout (std430, binding=4) buffer vertexParents
{
  uint vertex_parents[];
};

layout (std430, binding=2) buffer patchTransform
{
  float patch_transform[];
};

layout (std430, binding=5) buffer patchTexTransform
{
  float patch_tex_transform[];
};

layout (std430, binding=3) buffer patchTessLevels
{
  float patch_tessLevels[];
};


uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

uniform sampler2D heightmap;
uniform float heightScale;
uniform float LODfactor;

uniform uint patchLevel;

in uint vtx_ID;
in uint patch_ID;


//*
out FragData {
  vec3 viewDirection;
  vec2 texcoord;
  float tessLevel;
  float distance;
} vs_to_fs;
//*/

vec3 vertexBarycentricPosition(uint vertexID){
  return vec3(vertex_data[vertexID*4], vertex_data[vertexID*4+1], vertex_data[vertexID*4+2]);
}

uint vertexBorder(uint vertexID){
  return uint(vertex_data[vertexID*4+3]);
}

float vertexTessLevel(uint patchID, uint vertexID){
  //TODO  for later when we have outer levels
  //return patch_tessLevels[patchID*4+vertexBorder(vertexID)];
  return patch_tessLevels[patchID];
}

mat3 patchTransformationMatrix(uint patchID){
  return mat3(vec2(patch_transform[patchID*6], 0), patch_transform[patchID*6+1],
	      vec2(patch_transform[patchID*6+2], 0), patch_transform[patchID*6+3],
	      vec2(patch_transform[patchID*6+4], 0), patch_transform[patchID*6+5]);		   
}

mat3x2 patchTexTransformationMatrix(uint patchID){
  return mat3x2(vec2(patch_tex_transform[patchID*6], patch_tex_transform[patchID*6+1]),
		vec2(patch_tex_transform[patchID*6+2], patch_tex_transform[patchID*6+3]),
		vec2(patch_tex_transform[patchID*6+4], patch_tex_transform[patchID*6+5]));
}

bool hasParents(uint vertexID){
  return vertex_parents[vertexID*2] != 0;
}

uint vertexParent(uint vertexID, uint second){
  return vertex_parents[vertexID*2+second]-1;
}

vec3 vertexPosition(uint vertexID, mat3 patchTransformation, mat3x2 patchTexTransformation){
  //in this example pos.xz are the uv coordinates
  vec3 baryPos = vertexBarycentricPosition(vertexID);
  vec3 pos = patchTransformation*baryPos;
  vec2 UV = patchTexTransformation*baryPos;
  pos.y = texture(heightmap, vec2(UV.x, UV.y)).r * heightScale;
  return pos;
}

float tessLevelConversion[] = float[](1, 2, 4, 8, 16, 32, 64);


void main()
{
  mat3 patchTransformation = patchTransformationMatrix(patch_ID);
  mat3x2 patchTexTransformation = patchTexTransformationMatrix(patch_ID);

  vec3 pos = vertexPosition(vtx_ID, patchTransformation, patchTexTransformation);

  float tessLevel = vertexTessLevel(patch_ID, vtx_ID);
  // 
  // tessLevel = LODfactor*(tessLevel-prevLevel) + prevLevel;
  if(patchLevel != 0 && hasParents(vtx_ID)){
    float prevLevel = tessLevelConversion[patchLevel-1];
    float interpolation = (tessLevel-prevLevel)/(tessLevelConversion[patchLevel]-prevLevel);

    vec3 p0 = vertexPosition(vertexParent(vtx_ID, 0), patchTransformation, patchTexTransformation);
    vec3 p1 = vertexPosition(vertexParent(vtx_ID, 1), patchTransformation, patchTexTransformation);

    // //we take the midpoint between the parents
    vec3 prevPos = mix(p0, p1, 0.5);

    pos = mix(prevPos, pos, interpolation);
  }
  
  gl_Position = projection*view*vec4(pos, 1.0);

  vs_to_fs.viewDirection = -gl_Position.xyz;
  vs_to_fs.texcoord = patchTexTransformation * vertexBarycentricPosition(vtx_ID);
  vs_to_fs.tessLevel = tessLevel;
  vs_to_fs.distance = length((view*model*vec4(pos,1.)).xyz);
}
