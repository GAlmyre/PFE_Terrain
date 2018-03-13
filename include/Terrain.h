#ifndef TERRAIN_H
#define TERRAIN_H

#include <QOpenGLShaderProgram>
#include <QImage>
#define cimg_display 0
#include "CImg/CImg.h"
#include "surface_mesh/surface_mesh.h"

#include "Mesh.h"

#define NB_TESS_LEVELS 7

class Terrain
{
public:
  Terrain();

  void init();

  void setHeightMap(const QString &filename);
  //void setHeightMap(const QImage& heightmap);
  void setTexture(const QImage&  texture);
  void setHeightScale(float heightScale);

  Eigen::Vector2f getSize();
  float getTriEdgeSize();
  float heightScale();

  float getHeight(float x, float z, bool scaled = true);
  bool coordsInTerrain(float x, float z);

  void draw(QOpenGLShaderProgram &shader);
  void drawHardwareTessellation(QOpenGLShaderProgram &shader);
  void drawPatchInstanciation(QOpenGLShaderProgram &shader);

  bool intersect(Eigen::Vector3f orig, Eigen::Vector3f dir, float &t);

  const QImage &heightmap();

  void updateBaseMesh();
  void clean();

  void createGrid(float width, float height, unsigned int nbCols, unsigned int nbRows, bool quads);
  void generateTessellatedPatches();
  
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

  cimg_library::CImg<float> _heightMapCImg;
  GLuint _heightMapGL;

  unsigned int _pixelsPerPatch;
  bool _quadPatches;
  int _width, _height, _rows, _cols;
  float _heightScale;


  //##### instanciation #####

  unsigned int _nbPatchs = 0;
  
  std::vector<unsigned int> _patchNbElements;//filled
  std::vector<float> _instPatchTessLevels;
  std::vector<float> _instVertexPositions;//filled
  std::vector<unsigned int> _instVertexParents;//filled
  std::vector<float> _instPatchTransform;  //filled
  std::vector<float> _instPatchTexTransform; //filled

  GLuint _instVertexPositionsSSBO;//init filled
  GLuint _instVertexParentsSSBO;//init filled
  GLuint _instPatchTransformSSBO;//init filled
  GLuint _instPatchTexTransformSSBO;//init filled
  GLuint _instPatchTessLevelsSSBO;//init 

  bool _needPatchTransformSSBOUpdate;

  QOpenGLVertexArrayObject _instVertexArray;
  QOpenGLBuffer * _instVertexIDBuffer[NB_TESS_LEVELS]; //created and filled
  std::vector<unsigned int> _instPatchID[NB_TESS_LEVELS]; //created filled
  QOpenGLBuffer * _instPatchIDBuffer[NB_TESS_LEVELS];//created and filled
  
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
