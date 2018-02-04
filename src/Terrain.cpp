#include <iostream>

#include "Terrain.h"

Terrain::Terrain()
  : _pixelsPerPatch(64), _quadPatches(false)
{
  updateBaseMesh();
}

void Terrain::setHeightMap(const QImage& heightMap)
{
  _heightMap = heightMap;
  updateBaseMesh();
}

void Terrain::setTexture(const QImage& texture)
{
  _texture = texture;
}

void Terrain::drawBaseMesh(QOpenGLShaderProgram &shader)
{
  _baseMesh.draw(shader);
}
void Terrain::drawHardwareTessellation()
{
  std::cout << "Terrain::drawHardwareTessellation not implemented yet." << std::endl;
}
void Terrain::drawPatchInstanciation()
{
  std::cout << "Terrain::drawPatchInstanciation not implemented yet." << std::endl;
}

//updates the base mesh grid with the correct size according to the height map's size and the patchSize
void Terrain::updateBaseMesh()
{
  //if we already loaded a height map
  if(!_heightMap.isNull())
    {
      int w,h,gridw, gridh;
      w = _heightMap.width();
      h = _heightMap.height();
      gridw = w/_pixelsPerPatch;
      gridh = h/_pixelsPerPatch;
      if(w%_pixelsPerPatch)
	gridw++;
      if(h%_pixelsPerPatch)
	gridh++;
      _baseMesh.createGrid(w, h, gridw, gridh, false);
    }
  else
    {
      //for debug purpose only we create a grid without height map
      //should clear the mesh instead
      _baseMesh.createGrid(5,5,10,10, false);
    }
}
