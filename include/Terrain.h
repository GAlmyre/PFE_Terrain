#ifndef TERRAIN_H
#define TERRAIN_H

#include <QOpenGLShaderProgram>
#include <QImage>

#include "surface_mesh/surface_mesh.h"

#include "Mesh.h"

class Terrain : public Mesh
{

 public:
  Terrain();

  void setHeightMap(const QImage& heightmap);
  void setTexture(const QImage&  texture);
  void setHeightScale(float heightScale);

  Eigen::Vector2f getSize();
  float getTriEdgeSize();
  float heightScale();

  float getHeight(float x, float z);
  bool coordsInTerrain(float x, float z);

  void draw(QOpenGLShaderProgram &shader);
  void drawHardwareTessellation(QOpenGLShaderProgram &shader);
  void drawPatchInstanciation();

  bool intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float &t);

  const QImage &heightmap();

  void updateBaseMesh();
  void clean();

 private:
  QOpenGLTexture *_heightMap = nullptr;
  QOpenGLTexture *_texture = nullptr;
  QImage _heightMapImage;

  unsigned int _pixelsPerPatch;
  bool _quadPatches;
  int _width, _height, _rows, _cols;
  float _heightScale;

 public:
  void createGrid(float width, float height, unsigned int nb_rows, unsigned int nb_columns, bool quads);
 private:
  void fillMeshBuffers();

  //hardware tessellation
  surface_mesh::Surface_mesh _baseMesh;

  //instanciation of tessellated patches
  /*
    list of preprocessed tessellated patches levels
   */
};

#endif //TERRAIN_H
