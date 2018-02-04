#ifndef TERRAIN_H
#define TERRAIN_H

#include <QOpenGLShaderProgram>
#include <QImage>

#include "Mesh.h"

class Terrain
{

 public:
  Terrain();

  void setHeightMap(const QImage& heightmap);
  void setTexture(const QImage&  texture);

  void drawBaseMesh(QOpenGLShaderProgram &shader);
  void drawHardwareTessellation();
  void drawPatchInstanciation();

  void updateBaseMesh();
  
 private:
  //TODO replace int by the correct type we will use for these
  QImage _heightMap;
  QImage _texture;
  unsigned int _pixelsPerPatch;
  bool _quadPatches;

  //hardware tessellation
  Mesh _baseMesh;

  //instanciation of tessellated patches
  /*
    list of preprocessed tessellated patches levels
   */
};

#endif //TERRAIN_H
