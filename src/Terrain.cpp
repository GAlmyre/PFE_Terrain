#include <iostream>

#include "Terrain.h"
#include "OpenGL.h"
#include "utils.h"

using namespace surface_mesh;
using namespace Eigen;

Terrain::Terrain()
  : _pixelsPerPatch(64), _quadPatches(false), _heightMap(nullptr), _texture(nullptr),
    _width(0), _height(0), _rows(0), _cols(0), _heightScale(50.f)
{
  //updateBaseMesh();
}

void Terrain::init() {
  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  f->glGenVertexArrays(1, &_vao);
  f->glGenBuffers(1, &_vbo);
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
  Surface_mesh::Face_property<float> maxElevation = _baseMesh.face_property<float>("f:maxElevation");

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
}

void Terrain::clean() {
  if (_heightMap) delete _heightMap;
  if (_texture) delete _texture;

  GLFuncs *f = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();
  f->glDeleteVertexArrays(1, &_vao);
  f->glDeleteBuffers(1, &_vbo);
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

float Terrain::getHeight(float x, float z, bool scaled) {
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

  return scaled ? P * _heightScale / 255.f : P;
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