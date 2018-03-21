#include <iostream>
#include <limits>

#include "Terrain.h"
#include "OpenGL.h"
#include "utils.h"

using namespace surface_mesh;
using namespace Eigen;

using namespace cimg_library;

Terrain::Terrain()
  : _pixelsPerPatch(64), _quadPatches(false), _heightMap(nullptr), _texture(nullptr),
    _width(0), _height(0), _rows(0), _cols(0), _heightScale(50.f), _heightFactor(50.f)
{}

void Terrain::init() {
  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  f->glGenVertexArrays(1, &_vao);
  f->glGenBuffers(1, &_vbo);

  generateTessellatedPatches();

  f->glGenBuffers(1, &_instVertexPositionsSSBO);
  f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _instVertexPositionsSSBO);
  f->glBufferData(GL_SHADER_STORAGE_BUFFER, _instVertexPositions.size()*sizeof(float), _instVertexPositions.data(), GL_STATIC_COPY);
  f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, _instVertexPositionsSSBO);

  f->glGenBuffers(1, &_instVertexParentsSSBO);
  f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _instVertexParentsSSBO);
  f->glBufferData(GL_SHADER_STORAGE_BUFFER, _instVertexParents.size()*sizeof(float), _instVertexParents.data(), GL_STATIC_COPY);
  f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 4, _instVertexParentsSSBO);

  f->glGenBuffers(1, &_instPatchTransformSSBO);
  f->glGenBuffers(1, &_instPatchTexTransformSSBO);
  f->glGenBuffers(1, &_instPatchTessLevelsSSBO);

  //initialize patchIDBuffers
  for(int i=0; i<NB_TESS_LEVELS; ++i){
    _instPatchIDBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _instPatchIDBuffer[i]->create();
    _instPatchIDBuffer[i]->bind();
    _instPatchIDBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
    _instPatchIDBuffer[i]->release();
  }

  _instVertexArray.create();

}

void Terrain::setHeightMap(const QString &filename) {
  _heightMapCImg.assign(filename.toStdString().c_str());

  bool is16bits = _heightMapCImg.max() > 255.f;
  float ushortMax = std::numeric_limits<unsigned short>::max();
  float ucharMax = std::numeric_limits<unsigned char>::max();
  float max = is16bits ? ushortMax : ucharMax;
  float half = (max + 1.f) / 2.f;

  // Manually compute gradient
  CImg<float> gradX(_heightMapCImg.width(), _heightMapCImg.height(), 1, 1);
  CImg<float> gradZ(_heightMapCImg.width(), _heightMapCImg.height(), 1, 1);

  CImg_3x3(I,float);
  cimg_for3x3(_heightMapCImg,x,y,0,0,I,float) {
      gradX(x, y, 0) = (Ipc - Inc) / 2.f + half;
      gradZ(x, y, 0) = (Icp - Icn) / 2.f + half;
    }

  _heightMapCImg.append(gradX, 'c');
  _heightMapCImg.append(gradZ, 'c');

  // Heightmap normalization to 16 bits
  if (!is16bits) _heightMapCImg = _heightMapCImg / ucharMax * ushortMax;
  // Set openGL orientation convention
  _heightMapCImg.mirror('y');

  CImg<unsigned short> glHeightMap(_heightMapCImg);
  // Interleave Data
  glHeightMap.permute_axes("cxyz");

  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  f->glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

  _heightMap = new QOpenGLTexture(QOpenGLTexture::Target2D);
  _heightMap->setSize(_heightMapCImg.width(), _heightMapCImg.height());
  _heightMap->setMipLevels(_heightMap->maximumMipLevels());
  _heightMap->setFormat(QOpenGLTexture::RGB16_UNorm);
  _heightMap->allocateStorage(QOpenGLTexture::RGB, QOpenGLTexture::UInt16);
  _heightMap->setData(QOpenGLTexture::RGB, QOpenGLTexture::UInt16, glHeightMap.data());

  _heightMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  _heightMap->setMagnificationFilter(QOpenGLTexture::Linear);
  _heightMap->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
  _heightMap->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);

  f->glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

  // Normalize heightmap to [0, 1]
  _heightMapCImg /= ushortMax;

  updateBaseMesh();
}

void Terrain::setTexture(const QImage& texture)
{
  _texture = new QOpenGLTexture(texture.mirrored());
  _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  _texture->setMagnificationFilter(QOpenGLTexture::Linear);
  _texture->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
  _texture->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);
}

Eigen::Vector2f Terrain::getSize() {
  return Eigen::Vector2f(_width, _height);
}

float Terrain::getTriEdgeSize() {
  return (float) _width / (float) _cols;
}

void Terrain::draw(QOpenGLShaderProgram &shader){
  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  shader.setUniformValue(shader.uniformLocation("model"), QMatrix4x4());
  if(_heightMap) _heightMap->bind(0);
  if(_texture)   _texture->bind(1);
  shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  shader.setUniformValue(shader.uniformLocation("texturemap"), 1);

  f->glBindVertexArray(_vao);
  f->glDrawArrays(GL_TRIANGLES, 0, _rows * _cols * 6);

  f->glBindVertexArray(0);
  if(_heightMap) _heightMap->release();
  if(_texture) _texture->release();
}

void Terrain::drawHardwareTessellation(QOpenGLShaderProgram &shader)
{
  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  // Set uniforms
  shader.setUniformValue(shader.uniformLocation("model"), QMatrix4x4());
  if(_heightMap) _heightMap->bind(0);
  if(_texture)   _texture->bind(1);
  shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  shader.setUniformValue(shader.uniformLocation("texturemap"), 1);

  f->glBindVertexArray(_vao);
  f->glDrawArrays(GL_PATCHES, 0, _rows * _cols * 6);

  f->glBindVertexArray(0);
  if(_heightMap) _heightMap->release();
  if(_texture)   _texture->release();
}

void Terrain::computeTessellationLevels(const Matrix4f &MVP, const Vector2f &viewport, float factor){
  Surface_mesh::Vertex_property<Vector3f> point = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Face_property<float> maxElevation = _baseMesh.face_property<float>("f:maxElevation");
  _instPatchTessLevels.clear();
  
  for(Surface_mesh::Face f : _baseMesh.faces()){
    Surface_mesh::Halfedge h = _baseMesh.halfedge(f);
    Vector3f v0, v1, v2;
    v0 = point[_baseMesh.to_vertex(h)];
    h = _baseMesh.next_halfedge(h);
    v1 = point[_baseMesh.to_vertex(h)];
    h = _baseMesh.next_halfedge(h);
    v2 = point[_baseMesh.to_vertex(h)];

    Vector3f centerEdge0 = v1 + (v2 - v1) / 2.f;
    Vector3f centerEdge1 = v2 + (v0 - v2) / 2.f;
    Vector3f centerEdge2 = v0 + (v1 - v0) / 2.f;

    // Triangle Barycenter
    Vector3f center = 2.f / 3.f * (centerEdge0 - v0) + v0;
    float elevation = maxElevation[f] * _heightFactor;

    Vector4f center4f(center.x(), center.y(), center.z(), 1.f);
    Vector4f clip0 = MVP * center4f;
    Vector4f clip1 = MVP * (center4f + Vector4f(0,elevation,0,0.f));//Vector4f(center.x(), center.y() + elevation, center.z(), 1.f);

    
    Vector2f ndc0 = Vector2f(clip0.x(), clip0.y()).array() / clip0.w() * viewport.array() * 0.5f;
    Vector2f ndc1 = Vector2f(clip1.x(), clip1.y()).array() / clip1.w() * viewport.array() * 0.5f;
    
    float dist = (ndc1-ndc0).norm();

    _instPatchTessLevels.push_back(std::max(0.f, std::min(dist, 64.f))*factor);
    //std::cout << "f : " << f.idx() << " lvl : " << std::max(0.f, std::min(dist, 64.f)) << std::endl;
    
  }
}

void Terrain::drawPatchInstanciation(QOpenGLShaderProgram &shader)
{
  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();
  // Set uniforms
  shader.setUniformValue(shader.uniformLocation("model"), QMatrix4x4());
  if(_heightMap) _heightMap->bind(0);
  if(_texture)   _texture->bind(1);
  shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  shader.setUniformValue(shader.uniformLocation("texturemap"), 1);

  //compute patch tess levels TODO
  // _instPatchTessLevels.clear();
  // for(unsigned int i=0; i<_nbPatchs; ++i){
  //   //_instPatchTessLevels.push_back(64);
  // }


  //we fill the patchID buffers
  for(unsigned int i=0; i<NB_TESS_LEVELS; ++i){
    _instPatchID[i].clear();
  }
  float levels[7] = {1, 2, 4, 8, 16, 32, 64};
  for(unsigned int i=0; i<_nbPatchs; ++i){
    float level = _instPatchTessLevels[i];
    for(unsigned int j=0; j<7; ++j){
      if(level <= levels[j]){
	_instPatchID[j].push_back(i);
	break;
      }
    }
  }

  if(_needPatchTransformSSBOUpdate){
    f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _instPatchTransformSSBO);
    f->glBufferData(GL_SHADER_STORAGE_BUFFER, _instPatchTransform.size()*sizeof(float), _instPatchTransform.data(), GL_DYNAMIC_COPY);
    f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, _instPatchTransformSSBO);
    std::cout << "update patch transform" << std::endl;
    f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _instPatchTexTransformSSBO);
    f->glBufferData(GL_SHADER_STORAGE_BUFFER, _instPatchTexTransform.size()*sizeof(float), _instPatchTexTransform.data(), GL_DYNAMIC_COPY);
    f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 5, _instPatchTexTransformSSBO);
    std::cout << "allocate : " << _nbPatchs << std::endl;
    for(unsigned int patchIt=0; patchIt < NB_TESS_LEVELS; ++patchIt){
      _instPatchIDBuffer[patchIt]->bind();
      _instPatchIDBuffer[patchIt]->allocate(_instPatchID[patchIt].data(), sizeof(unsigned int)*_nbPatchs);
    }
    _needPatchTransformSSBOUpdate = false;
  }
  //std::cout << "display" << std::endl;

  //we copy patch tessellation levels to the ssbo
  f->glBindBuffer(GL_SHADER_STORAGE_BUFFER, _instPatchTessLevelsSSBO);
  f->glBufferData(GL_SHADER_STORAGE_BUFFER, _instPatchTessLevels.size()*sizeof(float), _instPatchTessLevels.data(), GL_DYNAMIC_COPY);
  f->glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 3, _instPatchTessLevelsSSBO);

  for(int patchIt = 0; patchIt < NB_TESS_LEVELS; ++patchIt){
    //std::cout << "test" << std::endl;

    _instVertexArray.bind();
    _instVertexIDBuffer[patchIt]->bind();

    int vertexID_loc = shader.attributeLocation("vtx_ID");
    if(vertexID_loc>=0) {
      f->glVertexAttribIPointer(vertexID_loc, 1, GL_UNSIGNED_INT, 0, 0);
      shader.enableAttributeArray(vertexID_loc);
    }

    _instPatchIDBuffer[patchIt]->bind();

    _instPatchIDBuffer[patchIt]->write(0, _instPatchID[patchIt].data(), sizeof(unsigned int) * _instPatchID[patchIt].size());
    //TODO check if it is more expensive to use allocate than write : if so use allocate after each createGrid call to change the allocation according to the number of patchs
    //_instPatchIDBuffer[patchIt]->allocate(_instPatchID[patchIt].data(), sizeof(unsigned int)*_instPatchID[patchIt].size());

    int patchID_loc = shader.attributeLocation("patch_ID");
    if(patchID_loc>=0) {
      f->glVertexAttribIPointer(patchID_loc, 1, GL_UNSIGNED_INT, 0, 0);
      shader.enableAttributeArray(patchID_loc);
    }

    f->glVertexAttribDivisor(vertexID_loc, 0);
    f->glVertexAttribDivisor(patchID_loc, 1);

    f->glUniform1ui(shader.uniformLocation("patchLevel"), patchIt);

    //std::cout << "test test" << _patchNbElements[patchIt] << " " << _instPatchID[patchIt].size() << std::endl;


    f->glDrawArraysInstanced(GL_TRIANGLES, 0, _patchNbElements[patchIt], _instPatchID[patchIt].size());



    if(vertexID_loc)
      shader.disableAttributeArray(vertexID_loc);
    _instVertexIDBuffer[patchIt]->release();
    _instVertexArray.release();

    //std::cout << "test end" << std::endl;
  }
  _heightMap->release();

}

//updates the base mesh grid with the correct size according to the height map's size and the patchSize
void Terrain::updateBaseMesh()
{
  //if we already loaded a height map
  if(!_heightMapCImg.is_empty())
  {
    _width = _heightMapCImg.width();
    _height = _heightMapCImg.height();
    _cols = _width / _pixelsPerPatch;
    _rows = _height / _pixelsPerPatch;
    _heightFactor = _heightScale * getSize().norm() / 900.f;

    if(_width % _pixelsPerPatch)
      _cols++;
    if(_height % _pixelsPerPatch)
      _rows++;

    createGrid(_width,_height, _cols, _rows, false);
  } else {
    //for debug purpose only we create a grid without height map
    //should clear the mesh instead
    _width = 5;
    _height = 5;
    _rows = 10;
    _cols = 10;
    _heightFactor = _heightScale * getSize().norm() / 900.f;

    createGrid(_width, _height, _cols, _rows, false);
  }
}

void Terrain::createGrid(float width, float height, unsigned int nbCols, unsigned int nbRows, bool quads)
{
  Surface_mesh::Vertex_property<Vector3f> points = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Vertex_property<Vector3f> normals = _baseMesh.vertex_property<Vector3f>("v:normal");
  Surface_mesh::Vertex_property<Vector2f> texcoords  = _baseMesh.vertex_property<Vector2f>("v:texcoords");
  Surface_mesh::Face_property<float> maxElevation = _baseMesh.face_property<float>("f:maxElevation");

  _baseMesh.clear();
  _instPatchTransform.clear();
  _instPatchTexTransform.clear();

  int nbVertices = (nbCols+1)*(nbRows+1);
  int nbFaces = nbCols*nbRows;
  int nbEdges = nbCols*(nbRows+1)+nbRows*(nbCols+1);
  if(!quads){
    nbEdges += nbFaces;//we add one edge per quad to divide it in two triangles
    nbFaces *= 2;
  }

  _nbPatchs = nbFaces;
  std::cout << "nb faces : " << nbFaces << std::endl;

  _baseMesh.reserve(nbVertices, nbEdges, nbFaces);

  Surface_mesh::Vertex v1, v2, v3, v4;
  Surface_mesh::Vertex * verts = new Surface_mesh::Vertex[nbVertices];

  //1st step : creating all the vertices
  int cpt = 0;
  for(int i=0; i < nbCols+1; ++i) {
    for (int j = 0; j < nbRows + 1; ++j) {
      float normPos_i = (float) i / nbCols, normPos_j = (float) j / nbRows;
      v1 = _baseMesh.add_vertex(Vector3f(normPos_i * width, 0, normPos_j * height));
      normals[v1] = Vector3f(normPos_i * width, 0, normPos_j * height);
      texcoords[v1] = Vector2f(normPos_i, normPos_j);
      verts[cpt++] = v1;
    }
  }

  float patchWidth = width / nbCols;
  float patchHeight = height / nbRows;
  float slope = - (float) patchHeight / (float) patchWidth;
  //2nd step : creating all the faces
  int lineOffset = -1;
  for(int i=0; i < nbCols*nbRows; ++i){
    if(i%nbRows==0)
      lineOffset++;
    int i1, i2, i3, i4;
    /* we get p1 i2 p3 p4 the indices of the vertices of the ith quad patch
       v2 ---- v4
       | `     |
       |   `   |
       |     ` |
       v1 ---- v3
    */
    i1 = i+lineOffset; i2 = i1+1;
    i3 = i1+nbRows+1;  i4 = i3+1;
    v1 = verts[i1];    v2 = verts[i2];
    v3 = verts[i3];    v4 = verts[i4];

    if(quads){
      _baseMesh.add_quad(v1, v3, v4, v2);
    } else {
      //split the quad into two triangles
      Surface_mesh::Face f0 = _baseMesh.add_triangle(v1, v3, v2);
      Surface_mesh::Face f1 = _baseMesh.add_triangle(v2, v3, v4);
      Vector3f pos1, pos2, pos3, pos4;
      Vector2f t1, t2, t3, t4;
      pos1 = points[v1]; pos2 = points[v2]; pos3 = points[v3]; pos4 = points[v4];
      t1 = texcoords[v1]; t2 = texcoords[v2]; t3 = texcoords[v3]; t4 = texcoords[v4];
      //*
      _instPatchTransform.push_back(pos1.x());
      _instPatchTransform.push_back(pos1.z());
      _instPatchTransform.push_back(pos3.x());
      _instPatchTransform.push_back(pos3.z());
      _instPatchTransform.push_back(pos2.x());
      _instPatchTransform.push_back(pos2.z());

      _instPatchTransform.push_back(pos2.x());
      _instPatchTransform.push_back(pos2.z());
      _instPatchTransform.push_back(pos3.x());
      _instPatchTransform.push_back(pos3.z());
      _instPatchTransform.push_back(pos4.x());
      _instPatchTransform.push_back(pos4.z());

      _instPatchTexTransform.push_back(t1.x());
      _instPatchTexTransform.push_back(t1.y());
      _instPatchTexTransform.push_back(t3.x());
      _instPatchTexTransform.push_back(t3.y());
      _instPatchTexTransform.push_back(t2.x());
      _instPatchTexTransform.push_back(t2.y());

      _instPatchTexTransform.push_back(t2.x());
      _instPatchTexTransform.push_back(t2.y());
      _instPatchTexTransform.push_back(t3.x());
      _instPatchTexTransform.push_back(t3.y());
      _instPatchTexTransform.push_back(t4.x());
      _instPatchTexTransform.push_back(t4.y());
      //*/
      /* Here, we will compute the lod based on the heightmap elevation in the triangles */
      Vector3f p1 = points[v1]; p1 += getHeight(p1.x(), p1.z(), false) * Vector3f::UnitY();
      Vector3f p2 = points[v2]; p2 += getHeight(p2.x(), p2.z(), false) * Vector3f::UnitY();
      Vector3f p3 = points[v3]; p3 += getHeight(p3.x(), p3.z(), false) * Vector3f::UnitY();
      Vector3f p4 = points[v4]; p4 += getHeight(p4.x(), p4.z(), false) * Vector3f::UnitY();

      // Planes corresponding to the 2 faces
      Hyperplane<float, 3> plane0 = Hyperplane<float, 3>::Through(p1, p2, p3);
      Hyperplane<float, 3> plane1 = Hyperplane<float, 3>::Through(p2, p3, p4);

      /* Compute heightmap max elevation above the face planes */
      float x1 = p1.x(), z1 = p1.z();
      float f0MaxElevation = 0.f, f1MaxElevation = 0.f;

      for (float x = 0.f; x < patchWidth; x += 1.f) {
        // Find the z range for the first triangle
        int zDiag = slope * x + patchHeight;
        // Triangle 0 rasterization
        for (int z = 0; z < zDiag; z += 1.f) {
          float h = getHeight(x + x1, z + z1, false);
          float hPlane = getY(plane0, x + x1, z + z1);
          f0MaxElevation = std::max(std::abs(h - hPlane), f0MaxElevation);
        }
        // Triangle 1 rasterization
        for (float z = zDiag; z < patchHeight; z++) {
          float h = getHeight(x + x1, z + z1, false);
          float hPlane = getY(plane1, x + x1, z + z1);
          f1MaxElevation = std::max(std::abs(h - hPlane), f1MaxElevation);
        }
      }
      maxElevation[f0] = f0MaxElevation;
      maxElevation[f1] = f1MaxElevation;
    }
  }

  delete[] verts;

  fillVertexArrayBuffer();

  _needPatchTransformSSBOUpdate = true;
}

void Terrain::generateTessellatedPatches(){

  Surface_mesh mesh;
  //mesh.reserve(1000,1000,1000);

  std::cout << "generate tess patches begin" << std::endl;
  // ##### Properties #####
  Surface_mesh::Vertex_property<Vector3f> point = mesh.get_vertex_property<Vector3f>("v:point");
  //tessellation level
  Surface_mesh::Vertex_property<unsigned int> vertexTessLevel = mesh.vertex_property<unsigned int>("v:tessLevel");
  Surface_mesh::Edge_property<unsigned int> edgeTessLevel = mesh.edge_property<unsigned int>("e:tessLevel");
  Surface_mesh::Face_property<unsigned int> faceTessLevel = mesh.face_property<unsigned int>("f:tessLevel");
  //vertex child
  Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_child = mesh.vertex_property<Surface_mesh::Vertex>("v:child");
  //edge child
  Surface_mesh::Edge_property<Surface_mesh::Vertex> edge_child = mesh.edge_property<Surface_mesh::Vertex>("e:child");
  //vertex parent1
  Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_parent1 = mesh.vertex_property<Surface_mesh::Vertex>("v:parent1");
  //vertex parent2
  Surface_mesh::Vertex_property<Surface_mesh::Vertex> vertex_parent2 = mesh.vertex_property<Surface_mesh::Vertex>("v:parent2");

  // ##### Tessellation levels creation loop #####

  for(int i=0; i<NB_TESS_LEVELS; ++i){
    //creation of the first level
    if(i==0){
      Surface_mesh::Vertex v1, v2, v3;
      v1 = mesh.add_vertex(Vector3f(1, 0, 0));
      v2 = mesh.add_vertex(Vector3f(0, 1, 0));
      v3 = mesh.add_vertex(Vector3f(0, 0, 1));
      Surface_mesh::Face f = mesh.add_triangle(v1, v2, v3);

      vertexTessLevel[v1] = vertexTessLevel[v2] = vertexTessLevel[v3] = faceTessLevel[f] = i;
      for(Surface_mesh::Halfedge h : mesh.halfedges(f)){
	Surface_mesh::Edge e = mesh.edge(h);
	edgeTessLevel[e] = i;
      }
    }
    //creation of every other levels
    else{
      //we make a copy of each vertex of the previous level and mark it as its child
      for(Surface_mesh::Vertex v : mesh.vertices()){
	if(vertexTessLevel[v] == i-1){
	  Surface_mesh::Vertex vChild = mesh.add_vertex(point[v]);
	  vertex_child[v] = vChild;

	  vertexTessLevel[vChild] = i;
	}
      }

      //we create a vertex at the middle of each edge of the previous level and mark it as its child
      for(Surface_mesh::Edge e : mesh.edges()){
	if(edgeTessLevel[e] == i-1){
	  Surface_mesh::Vertex ev0, ev1;
	  ev0 = mesh.vertex(e, 0);
	  ev1 = mesh.vertex(e, 1);
	  Surface_mesh::Vertex v = mesh.add_vertex((point[ev0] + point[ev1])/2);
	  edge_child[e] = v;
	  vertex_parent1[v] = ev0;
	  vertex_parent2[v] = ev1;

	  vertexTessLevel[v] = i;
	}
      }

      //we create 4 triangles for each
      for(Surface_mesh::Face f : mesh.faces()){
	if(faceTessLevel[f] == i-1){
	  Surface_mesh::Vertex v0, v1, v2, ev0, ev1, ev2;
	  Surface_mesh::Halfedge h = mesh.halfedge(f);
	  v0 = vertex_child[mesh.to_vertex(h)];
	  ev0 = edge_child[mesh.edge(h)];
	  h = mesh.next_halfedge(h);
	  v1 = vertex_child[mesh.to_vertex(h)];
	  ev1 = edge_child[mesh.edge(h)];
	  h = mesh.next_halfedge(h);
	  v2 = vertex_child[mesh.to_vertex(h)];
	  ev2 = edge_child[mesh.edge(h)];

	  Surface_mesh::Face f0, f1, f2, f3;
	  f0 = mesh.add_triangle(ev0, v0, ev1);
	  f1 = mesh.add_triangle(ev1, v1, ev2);
	  f2 = mesh.add_triangle(ev2, v2, ev0);
	  f3 = mesh.add_triangle(ev0, ev1, ev2);

	  faceTessLevel[f0] = i;
	  faceTessLevel[f1] = i;
	  faceTessLevel[f2] = i;
	  faceTessLevel[f3] = i;

	  for(Surface_mesh::Halfedge h : mesh.halfedges(f0)){
	    Surface_mesh::Edge e = mesh.edge(h);
	    edgeTessLevel[e] = i;
	  }

	  for(Surface_mesh::Halfedge h : mesh.halfedges(f1)){
	    Surface_mesh::Edge e = mesh.edge(h);
	    edgeTessLevel[e] = i;
	  }

	  for(Surface_mesh::Halfedge h : mesh.halfedges(f2)){
	    Surface_mesh::Edge e = mesh.edge(h);
	    edgeTessLevel[e] = i;
	  }

	  for(Surface_mesh::Halfedge h : mesh.halfedges(f3)){
	    Surface_mesh::Edge e = mesh.edge(h);
	    edgeTessLevel[e] = i;
	  }
	}
      }
    }
  }

  // ##### fill the tess level vertexIDBuffer with vertices indices
  std::vector<unsigned int> vertexID[NB_TESS_LEVELS];
  for(Surface_mesh::Face f : mesh.faces()){
    unsigned int level = faceTessLevel[f];
    for(Surface_mesh::Vertex v : mesh.vertices(f)){
      vertexID[level].push_back(v.idx());
    }
  }

  for(int i=0; i<NB_TESS_LEVELS; ++i){
    _patchNbElements.push_back(vertexID[i].size());

    _instVertexIDBuffer[i] = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    _instVertexIDBuffer[i]->create();
    _instVertexIDBuffer[i]->bind();
    _instVertexIDBuffer[i]->setUsagePattern(QOpenGLBuffer::StaticDraw);
    _instVertexIDBuffer[i]->allocate(vertexID[i].data(), sizeof(unsigned int)*vertexID[i].size());
    _instVertexIDBuffer[i]->release();
  }

  // ##### fill the vertexPosition and vertexParents vectors
  for(Surface_mesh::Vertex v : mesh.vertices()){
    Vector3f p = point[v];
    _instVertexPositions.push_back(p.x());
    _instVertexPositions.push_back(p.y());
    _instVertexPositions.push_back(p.z());
    //TODO 4th value is border value
    _instVertexPositions.push_back(0);

    //for parents  0 = no parent (idx = -1)   all idx are shifted by one to fit in an unsigned int
    _instVertexParents.push_back(vertex_parent1[v].idx()+1);
    _instVertexParents.push_back(vertex_parent2[v].idx()+1);
  }

  std::cout << "generate tess patches end" << std::endl;
}

void Terrain::clean() {
  delete _heightMap;
  delete _texture;

  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();
  f->glDeleteVertexArrays(1, &_vao);
  f->glDeleteBuffers(1, &_vbo);

  //TODO add delete for instanciation
}

bool Terrain::intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float &tHit) {
  Vector3f invDir = dir.cwiseInverse();
  const Vector3f min(0.f, 0.f, 0.f);
  const Vector3f max(_width, _heightFactor, _height);

  /* First, we find the intersection between the ray and the terrain bounding box */
  float tmin, tmax, tymin, tymax, tzmin, tzmax;

  if (invDir.x() >= 0) {
    tmin = (min.x() - orig.x()) * invDir.x();
    tmax = (max.x() - orig.x()) * invDir.x();
  }
  else {
    tmin = (max.x() - orig.x()) * invDir.x();
    tmax = (min.x() - orig.x()) * invDir.x();
  }

  if (invDir.y() >= 0) {
    tymin = (min.y() - orig.y()) * invDir.y();
    tymax = (max.y() - orig.y()) * invDir.y();
  }
  else {
    tymin = (max.y() - orig.y()) * invDir.y();
    tymax = (min.y() - orig.y()) * invDir.y();
  }

  if ((tmin > tymax) || (tymin > tmax))
    return false;
  if (tymin > tmin)
    tmin = tymin;
  if (tymax < tmax)
    tmax = tymax;

  if (invDir.z() >= 0) {
    tzmin = (min.z() - orig.z()) * invDir.z();
    tzmax = (max.z() - orig.z()) * invDir.z();
  }
  else {
    tzmin = (max.z() - orig.z()) * invDir.z();
    tzmax = (min.z() - orig.z()) * invDir.z();
  }

  if ((tmin > tzmax) || (tzmin > tmax))
    return false;
  if (tzmin > tmin)
    tmin = tzmin;
  if (tzmax < tmax)
    tmax = tzmax;

  if (tmin < 0) tmin = 0;

  /* We found an intersection with the AABB, let's find the intersection with the terrain by ray marching */
  float deltat = 0.1f;

  // If we intersect at the begining of the BBox, we are under the terrain, don't consider it as an intersection
  Vector3f p = orig + dir * (tmin + deltat);
  if (p.y() < _heightMapCImg((int) p.x(), (int) p.z(), 0) * _heightFactor)
    return false;

  for (float t = tmin + deltat; t <= tmax - deltat; t += deltat) {
    Vector3f p = orig + dir * t;
    float h = _heightMapCImg((int) p.x(), (int) p.z(), 0) * _heightFactor;
    if (p.y() < h) {
      tHit = t - 0.5 * deltat;
      return true;
    }
  }
  return false;
}

void Terrain::setHeightScale(float heightScale) {
  _heightScale = heightScale;
  _heightFactor = _heightScale * getSize().norm() / 900.f;
}

float Terrain::heightScale() {
  return _heightFactor;
}

float Terrain::getHeight(float x, float z, bool scaled) {
  float val = _heightMapCImg.linear_atXY(x, z);
  return scaled ? val * _heightFactor : val;
}

bool Terrain::coordsInTerrain(float x, float z) {
  return (x >= 0 && x <= _width - 1 && z >= 0 && z <= _height - 1);
}

void Terrain::fillVertexArrayBuffer() {
  std::vector<Vertex> vertices;
  vertices.reserve(_rows * _cols * 6);

  Surface_mesh::Vertex_property<Vector3f> positions = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Vertex_property<Vector2f> texcoords  = _baseMesh.vertex_property<Vector2f>("v:texcoords");
  Surface_mesh::Face_property<float> maxElevation = _baseMesh.face_property<float>("f:maxElevation");

  // face iterator
  Surface_mesh::Face_iterator fit, fend = _baseMesh.faces_end();
  // vertex circulator
  Surface_mesh::Vertex v0, v1, v2;
  for (fit = _baseMesh.faces_begin(); fit != fend; ++fit)
  {
    float faceLOD = maxElevation[*fit];

    Surface_mesh::Halfedge he0 = _baseMesh.halfedge(*fit);
    Surface_mesh::Halfedge he1 = _baseMesh.next_halfedge(he0);
    Surface_mesh::Halfedge he2 = _baseMesh.next_halfedge(he1);

    v0 = _baseMesh.from_vertex(he0);
    v1 = _baseMesh.from_vertex(he1);
    v2 = _baseMesh.from_vertex(he2);

    Surface_mesh::Face oppositef0 = _baseMesh.face(_baseMesh.opposite_halfedge(he1));
    Surface_mesh::Face oppositef1 = _baseMesh.face(_baseMesh.opposite_halfedge(he2));
    Surface_mesh::Face oppositef2 = _baseMesh.face(_baseMesh.opposite_halfedge(he0));

    float edgeLOD[3] = { faceLOD, faceLOD, faceLOD };
    if (oppositef0.is_valid()) {
      edgeLOD[0] = (edgeLOD[0] + maxElevation[oppositef0]) / 2.f;
    }
    if (oppositef1.is_valid()) {
      edgeLOD[1] = (edgeLOD[1] + maxElevation[oppositef1]) / 2.f;
    }
    if (oppositef2.is_valid()) {
      edgeLOD[2] = (edgeLOD[2] + maxElevation[oppositef2]) / 2.f;
    }

    vertices.emplace_back(positions[v0], texcoords[v0], edgeLOD[0], faceLOD);
    vertices.emplace_back(positions[v1], texcoords[v1], edgeLOD[1], faceLOD);
    vertices.emplace_back(positions[v2], texcoords[v2], edgeLOD[2], faceLOD);
  }

  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  f->glBindVertexArray(_vao);

  f->glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  f->glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), vertices.data(), GL_STATIC_DRAW);

  // Positions
  f->glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *) 0);
  f->glEnableVertexAttribArray(0);

  // TexCoords
  f->glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(offsetof(Vertex, texcoords)));
  f->glEnableVertexAttribArray(1);

  // Edge LOD
  f->glVertexAttribPointer(2, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(offsetof(Vertex, edgeLOD)));
  f->glEnableVertexAttribArray(2);

  // Face LOD
  f->glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid *)(offsetof(Vertex, faceLOD)));
  f->glEnableVertexAttribArray(3);

  f->glBindVertexArray(0);
}
