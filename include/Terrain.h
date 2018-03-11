#ifndef TERRAIN_H
#define TERRAIN_H

#include <QOpenGLShaderProgram>
#include <QImage>

#include "surface_mesh/surface_mesh.h"

#include "Mesh.h"

class Terrain
{
public:
  Terrain();

  void init();

  void setHeightMap(const QImage& heightmap);
  void setTexture(const QImage&  texture);
  void setHeightScale(float heightScale);

  Eigen::Vector2f getSize();
  float getTriEdgeSize();
  float heightScale();

  float getHeight(float x, float z, bool scaled = true);
  bool coordsInTerrain(float x, float z);

  void draw(QOpenGLShaderProgram &shader);
  void drawHardwareTessellation(QOpenGLShaderProgram &shader);
  void drawPatchInstanciation();

  bool intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float &t);

  const QImage &heightmap();

  void updateBaseMesh();
  void clean();

  void createGrid(float width, float height, unsigned int nbCols, unsigned int nbRows, bool quads);

private:
  struct Vertex {
    Vertex(const Eigen::Vector3f &position, const Eigen::Vector2f &texcoords, float edgeLOD, float faceLOD)
            : position(position), texcoords(texcoords), edgeLOD(edgeLOD), faceLOD(faceLOD) {}

    Eigen::Vector3f position;
    Eigen::Vector2f texcoords;
    float edgeLOD;
    float faceLOD;
  };
  std::vector<Vertex> _vertices;

  GLuint _vao;
  GLuint _vbo;

  QOpenGLTexture *_heightMap = nullptr;
  QOpenGLTexture *_texture = nullptr;
  QImage _heightMapImage;

  unsigned int _pixelsPerPatch;
  bool _quadPatches;
  int _width, _height, _rows, _cols;
  float _heightScale;
private:
  void fillVertexArrayBuffer();

  //hardware tessellation
  surface_mesh::Surface_mesh _baseMesh;



  //instanciation of tessellated patches
  /*
    list of preprocessed tessellated patches levels
   */
};

#endif //TERRAIN_H
