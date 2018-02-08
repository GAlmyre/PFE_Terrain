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
  
  void drawHardwareTessellation(QOpenGLShaderProgram &shader);
  void drawPatchInstanciation();

  void updateBaseMesh();
  void clean();
  
 private:
  QImage _heightMap;
  QImage _texture;
  QOpenGLTexture *_heightMapTexture;
  unsigned int _pixelsPerPatch;
  bool _quadPatches;

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
