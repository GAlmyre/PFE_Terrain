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

  Eigen::Vector2f getSize();
  float getTriEdgeSize();

  void draw(QOpenGLShaderProgram &shader);
  void drawHardwareTessellation(QOpenGLShaderProgram &shader);
  void drawPatchInstanciation();

  bool intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float heightScale, float &t);

  const QImage &heightmap();

  void updateBaseMesh();
  void clean();

 private:
  QOpenGLTexture *_heightMap = nullptr;
  QOpenGLTexture *_texture = nullptr;
  QImage _heightMapImage;
  Eigen::AlignedBox3f _bBox;
  unsigned int _pixelsPerPatch;
  bool _quadPatches;
  int _width, _height, _rows, _cols;

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
