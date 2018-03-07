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

layout (std430, binding=3) buffer patchTessLevels
{
  float patch_tessLevels[];
};


uniform mat4 projection;
uniform mat4 view;

uniform sampler2D heightmap;

uniform uint patchLevel;

in uint vtx_ID;
in uint patch_ID;

out vec3 v_color;

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

bool hasParents(uint vertexID){
  return vertex_parents[vertexID*2] != 0;
}

uint vertexParent(uint vertexID, uint second){
  return vertex_parents[vertexID*2+second]-1;
}

vec3 vertexPosition(uint vertexID, mat3 patchTransformation){
  //in this example pos.xz are the uv coordinates
  vec3 pos = patchTransformation*vertexBarycentricPosition(vertexID);
  pos.y = texture(heightmap, vec2(pos.x, pos.z)).r;
  return pos;
}

float tessLevelConversion[] = float[](1, 2, 4, 8, 16, 32, 64);


void main()
{
  //in this example we only have 2 patchs
  if(patch_ID == 0)
    v_color = vec3(1,0,0);
  if(patch_ID == 1)
    v_color = vec3(0,1,0);

  mat3 patchTransformation = patchTransformationMatrix(patch_ID);

  vec3 pos = vertexPosition(vtx_ID, patchTransformation);
  
  if(patchLevel != 0 && hasParents(vtx_ID)){
    float tessLevel = vertexTessLevel(patch_ID, vtx_ID);
    float prevLevel = tessLevelConversion[patchLevel-1];
    float interpolation = (tessLevel-prevLevel)/(tessLevelConversion[patchLevel]-prevLevel);

    vec3 p0 = vertexPosition(vertexParent(vtx_ID, 0), patchTransformation);
    vec3 p1 = vertexPosition(vertexParent(vtx_ID, 1), patchTransformation);

    // //we take the midpoint between the parents
    vec3 prevPos = mix(p0, p1, 0.5);

    pos = mix(prevPos, pos, interpolation);
    //pos = prevPos;
  }
  
  gl_Position = projection*view*vec4(pos, 1.0);
}
