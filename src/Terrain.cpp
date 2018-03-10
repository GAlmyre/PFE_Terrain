#include <iostream>

#include "Terrain.h"

using namespace surface_mesh;
using namespace Eigen;

Terrain::Terrain()
  : _pixelsPerPatch(64), _quadPatches(false), _heightMap(nullptr), _texture(nullptr),
    _width(0), _height(0), _rows(0), _cols(0), _heightScale(50.f)
{
  //updateBaseMesh();
}

void Terrain::setHeightMap(const QImage& heightMap)
{
  _heightMapImage = heightMap.mirrored();

  _heightMap = new QOpenGLTexture(_heightMapImage);
  _heightMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  _heightMap->setMagnificationFilter(QOpenGLTexture::Linear);
  _heightMap->setWrapMode(QOpenGLTexture::DirectionS, QOpenGLTexture::ClampToEdge);
  _heightMap->setWrapMode(QOpenGLTexture::DirectionT, QOpenGLTexture::ClampToEdge);

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
  if(_heightMap){
    _heightMap->bind(0);
  }
  if(_texture){
    _texture->bind(1);
  }
  shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  shader.setUniformValue(shader.uniformLocation("texturemap"), 1);
  Mesh::draw(shader);
  if(_heightMap)
    _heightMap->release();
  if(_texture)
    _texture->release();
}

void Terrain::drawHardwareTessellation(QOpenGLShaderProgram &shader)
{
//  std::cout << "Terrain::drawHardwareTessellation not implemented yet." << std::endl;
  if(!_initialized)
    initVAO();

  // Set uniforms
  QMatrix4x4 model;
  model.setToIdentity();

  shader.setUniformValue(shader.uniformLocation("model"), model);
  if(_heightMap)
    _heightMap->bind(0);
  if(_texture)
    _texture->bind(1);
  shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  shader.setUniformValue(shader.uniformLocation("texturemap"), 1);
  _vertexArray.bind();
  _vertexBuffer->bind();
  _indexBuffer->bind();

  int vertex_loc = shader.attributeLocation("vtx_position");
  if(vertex_loc>=0) {
    shader.setAttributeBuffer(vertex_loc, GL_FLOAT, offsetof(Mesh::Vertex,position), 3, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(vertex_loc);
  }

  int normal_loc = shader.attributeLocation("vtx_normal");
  if(normal_loc>=0) {
    shader.setAttributeBuffer(normal_loc, GL_FLOAT, offsetof(Mesh::Vertex,normal), 3, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(normal_loc);
  }

  int texcoord_loc = shader.attributeLocation("vtx_texcoord");
  if(texcoord_loc>=0) {
    shader.setAttributeBuffer(texcoord_loc, GL_FLOAT, offsetof(Mesh::Vertex,texcoord), 2, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(texcoord_loc);
  }

  glDrawElements(GL_PATCHES, _indices.size(), GL_UNSIGNED_INT, 0);

  _indexBuffer->release();
  _vertexBuffer->release();
  _vertexArray.release();

  if(_heightMap)
    _heightMap->release();
  if(_texture)
    _texture->release();
}
void Terrain::drawPatchInstanciation()
{
  std::cout << "Terrain::drawPatchInstanciation not implemented yet." << std::endl;
}

//updates the base mesh grid with the correct size according to the height map's size and the patchSize
void Terrain::updateBaseMesh()
{
  std::cout << "updateBaseMesh" << std::endl;
  //if we already loaded a height map
  if(_heightMap)
  {
    std::cout << "updateBaseMesh image loaded" << std::endl;
    _width = _heightMap->width();
    _height = _heightMap->height();
    _cols = _width / _pixelsPerPatch;
    _rows = _height / _pixelsPerPatch;

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
    createGrid(_width, _height, _cols, _rows, false);
  }
}

void Terrain::createGrid(float width, float height, unsigned int nbCols, unsigned int nbRows, bool quads)
{
  Surface_mesh::Vertex_property<Vector3f> points = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Vertex_property<Vector3f> normals = _baseMesh.vertex_property<Vector3f>("v:normal");
  Surface_mesh::Vertex_property<Vector2f> texcoords  = _baseMesh.vertex_property<Vector2f>("v:texcoords");
  Surface_mesh::Vertex_property<float> elevation = _baseMesh.vertex_property<float>("v:elevation");
  Surface_mesh::Face_property<int> maxElevation = _baseMesh.face_property<int>("f:maxElevation");

  _baseMesh.clear();

  int nbVertices = (nbCols+1)*(nbRows+1);
  int nbFaces = nbCols*nbRows;
  int nbEdges = nbCols*(nbRows+1)+nbRows*(nbCols+1);
  if(!quads){
    nbEdges += nbFaces;//we add one edge per quad to divide it in two triangles
    nbFaces *= 2;
  }

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
      elevation[v1] = getHeight(normPos_i * (width - 1), normPos_j * (height - 1));
      texcoords[v1] = Vector2f(normPos_i, normPos_j);
      verts[cpt++] = v1;
    }
  }

  int patchWidth = width / nbCols;
  int patchHeight = height / nbRows;
  float slope = - patchHeight / patchWidth;
  //2nd step : creating all the faces
  int lineOffset = -1;
  for(int i=0; i < nbCols*nbRows; ++i){
    if(i%nbRows==0)
      lineOffset++;
    int p1, p2, p3, p4;
    /* we get p1 p2 p3 p4 the indices of the vertices of the ith quad patch 
       p2 --- p4
       |      |
       |      |
       p1 --- p3
    */
    p1 = i+lineOffset; p2 = p1+1;
    p3 = p1+nbRows+1;  p4 = p3+1;
    v1 = verts[p1];    v2 = verts[p2];
    v3 = verts[p3];    v4 = verts[p4];
    if(quads){
      _baseMesh.add_quad(v1, v3, v4, v2);
    } else {
      //split the quad into two triangles
      Surface_mesh::Face f0 = _baseMesh.add_triangle(v1, v3, v2);
      Surface_mesh::Face f1 = _baseMesh.add_triangle(v2, v3, v4);

      int f0MeanElevation = (elevation[v1] + elevation[v2] + elevation[v3]) / 3.f;
      int f1MeanElevation = (elevation[v2] + elevation[v3] + elevation[v4]) / 3.f;

      /* Compute faces max elevation */
      int x1 = _baseMesh.position(v1).x();
      int y1 = _baseMesh.position(v1).z();

      int f0MaxElevation = 0, f1MaxElevation = 0;
      for (int x = 0; x < patchWidth; x++) {
        int yDiag = slope * x + patchHeight;
        for (int y = 0; y < yDiag; y++) {
          f0MaxElevation = std::max(qRed(_heightMapImage.pixel(x + x1, y + y1)), f0MaxElevation);
        }
        for (int y = yDiag; y < patchHeight; y++) {
          f1MaxElevation = std::max(qRed(_heightMapImage.pixel(x + x1, y + y1)), f1MaxElevation);
        }
      }
      maxElevation[f0] = f0MaxElevation - f0MeanElevation;
      maxElevation[f1] = f1MaxElevation - f1MeanElevation;
    }
  }

  delete[] verts;
  fillMeshBuffers();
}


void Terrain::fillMeshBuffers()
{
  _vertices.clear();
  _indices.clear();
  
  _initialized = false;
  Surface_mesh::Vertex_property<Vector3f> vertices = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Vertex_property<Vector3f> vnormals = _baseMesh.get_vertex_property<Vector3f>("v:normal");
  Surface_mesh::Vertex_property<Vector2f> texcoords = _baseMesh.get_vertex_property<Vector2f>("v:texcoords");
  
  Surface_mesh::Vertex_iterator vit;
  
  Vector3f pos;
  Vector3f normal;
  Vector2f tex;
  for(vit = _baseMesh.vertices_begin(); vit != _baseMesh.vertices_end(); ++vit)
  {
    pos = vertices[*vit];
    normal = vnormals[*vit];
    if(texcoords)
      tex = texcoords[*vit];
    _vertices.push_back(Mesh::Vertex(pos,normal,tex));
  }

  // face iterator
  Surface_mesh::Face_iterator fit, fend = _baseMesh.faces_end();
  // vertex circulator
  Surface_mesh::Vertex_around_face_circulator fvit, fvend;
  Surface_mesh::Vertex v0, v1, v2;
  for (fit = _baseMesh.faces_begin(); fit != fend; ++fit)
  {
    fvit = fvend = _baseMesh.vertices(*fit);
    v0 = *fvit;
    ++fvit;
    v2 = *fvit;

    do {
      v1 = v2;
      ++fvit;
      v2 = *fvit;
      _indices.push_back(v0.idx());
      _indices.push_back(v1.idx());
      _indices.push_back(v2.idx());
    } while (++fvit != fvend);
  }
}

void Terrain::clean() {
  Mesh::clean();
  if (_heightMap) delete _heightMap;
  if (_texture) delete _texture;
}

const QImage &Terrain::heightmap() {
  return _heightMapImage;
}

bool Terrain::intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float &tHit) {
  Vector3f invDir = dir.cwiseInverse();
  const Vector3f min(0.f, 0.f, 0.f);
  const Vector3f max(_width, _heightScale, _height);

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
  float factor = _heightScale / 255.f;
  float deltat = 0.1f;

  // If we intersect at the begining of the BBox, we are under the terrain, don't consider it as an intersection
  Vector3f p = orig + dir * (tmin + deltat);
  if (p.y() < qRed(_heightMapImage.pixel((int) p.x(), (int) p.z())) * factor)
    return false;

  for (float t = tmin + deltat; t <= tmax - deltat; t += deltat) {
    Vector3f p = orig + dir * t;
    float h = qRed(_heightMapImage.pixel((int) p.x(), (int) p.z())) * factor;
    if (p.y() < h) {
      tHit = t - 0.5 * deltat;
      return true;
    }
  }
  std::cout << "No intersection" << std::endl;
  return false;
}

void Terrain::setHeightScale(float heightScale) {
  _heightScale = heightScale;
}

float Terrain::heightScale() {
  return _heightScale;
}

float Terrain::getHeight(float x, float z) {
  int x1 = std::max(std::min(static_cast<int>(x), _width - 2), 0);
  int z1 = std::max(std::min(static_cast<int>(z), _height - 2), 0);

  float Q11 = qRed(_heightMapImage.pixel(x1,     z1));
  float Q21 = qRed(_heightMapImage.pixel(x1 + 1, z1));
  float Q12 = qRed(_heightMapImage.pixel(x1,     z1 + 1));
  float Q22 = qRed(_heightMapImage.pixel(x1 + 1, z1 + 1));

  float dx = std::min(std::max(x - x1, 0.f), 1.f);
  float dz = std::min(std::max(z - z1, 0.f), 1.f);

  float R1 = Q11 * (1.f - dx) + Q21 * dx;
  float R2 = Q12 * (1.f - dx) + Q22 * dx;

  float P = R1 * (1.f - dz) + R2 * dz;

  return P * _heightScale / 255.f;
}

bool Terrain::coordsInTerrain(float x, float z) {
  return (x >= 0 && x <= _width - 1 && z >= 0 && z <= _height - 1);
}

float Terrain::getHeightFromNormalizedCoords(float x, float z) {
}
