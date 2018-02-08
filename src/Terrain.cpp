#include <iostream>

#include "Terrain.h"

using namespace surface_mesh;
using namespace Eigen;

Terrain::Terrain()
  : _pixelsPerPatch(64), _quadPatches(false), _heightMap(nullptr), _texture(nullptr)
{
  updateBaseMesh();
}

void Terrain::setHeightMap(const QImage& heightMap)
{
  _heightMap = new QOpenGLTexture(heightMap.mirrored());
  _heightMap->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  _heightMap->setMagnificationFilter(QOpenGLTexture::Linear);
  updateBaseMesh();
}

void Terrain::setTexture(const QImage& texture)
{
  _texture = new QOpenGLTexture(texture.mirrored());
  _texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear);
  _texture->setMagnificationFilter(QOpenGLTexture::Linear);
}

void Terrain::draw(QOpenGLShaderProgram &shader){
  if(_heightMap){
    _heightMap->bind(0);
    shader.setUniformValue(shader.uniformLocation("heightmap"), 0);
  }
  Mesh::draw(shader);
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
  std::cout << "updateBaseMesh" << std::endl;
  //if we already loaded a height map
  if(_heightMap)
    {
      std::cout << "updateBaseMesh image loaded" << std::endl;
      int w,h,gridw, gridh;
      w = _heightMap->width();
      h = _heightMap->height();
      gridw = w/_pixelsPerPatch;
      gridh = h/_pixelsPerPatch;
      if(w%_pixelsPerPatch)
	gridw++;
      if(h%_pixelsPerPatch)
	gridh++;
      createGrid(w, h, gridw, gridh, false);
    }
  else
    {
      //for debug purpose only we create a grid without height map
      //should clear the mesh instead
      createGrid(5,5,10,10, false);
    }
}

void Terrain::createGrid(float width, float height, unsigned int nbRows, unsigned int nbColumns, bool quads)
{
  Surface_mesh::Vertex_property<Vector3f> points = _baseMesh.get_vertex_property<Vector3f>("v:point");
  Surface_mesh::Vertex_property<Vector3f> normals = _baseMesh.vertex_property<Vector3f>("v:normal");
  Surface_mesh::Vertex_property<Vector2f> texcoords  = _baseMesh.vertex_property<Vector2f>("v:texcoords");
  _baseMesh.clear();
  int nbVertices = (nbRows+1)*(nbColumns+1);
  int nbFaces = nbRows*nbColumns;
  int nbEdges = nbRows*(nbColumns+1)+nbColumns*(nbRows+1);
  if(!quads){
    nbEdges += nbFaces;//we add one edge per quad to divide it in two triangles
    nbFaces *= 2;
  }
  _baseMesh.reserve(nbVertices, nbEdges, nbFaces);
  Surface_mesh::Vertex v1, v2, v3, v4;
  Surface_mesh::Vertex * verts = new Surface_mesh::Vertex[nbVertices];

  //1st step : creating all the vertices
  int cpt = 0;
  for(int i=0; i < nbRows+1; ++i)
    for(int j=0; j < nbColumns+1; ++j){
      float normPos_i = (float)i/nbRows, normPos_j = (float)j/nbColumns;
      v1 = _baseMesh.add_vertex(Vector3f(normPos_i*width, 0, normPos_j*height));
      normals[v1] = Vector3f(normPos_i*width, 0, normPos_j*height);
      texcoords[v1] = Vector2f(normPos_i,normPos_j);
      verts[cpt++] = v1;   
    }
  //2nd step : creating all the faces
   int lineOffset = -1;
  for(int i=0; i < nbRows*nbColumns; ++i){
    if(i%nbColumns==0)
      lineOffset++;
    int p1, p2, p3, p4;
    /* we get p1 p2 p3 p4 the indices of the vertices of the ith quad patch 
       p1 --- p2
       |      |
       |      |
       p3 --- p4
    */
    p1 = i+lineOffset;    p2 = p1+1;
    p3 = p1+nbColumns+1;  p4 = p3+1;
    v1 = verts[p1];       v2 = verts[p2];
    v3 = verts[p3];       v4 = verts[p4];
    if(quads){
      _baseMesh.add_quad(v1, v3, v4, v2);
    }
    else{
      //split the quad into two triangles
      _baseMesh.add_triangle(v1, v3, v2);
      _baseMesh.add_triangle(v2, v3, v4);
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

      do{
	v1 = v2;
	++fvit;
	v2 = *fvit;
	_indices.push_back(v0.idx());
	_indices.push_back(v1.idx());
	_indices.push_back(v2.idx());
      } while (++fvit != fvend);
    }
}
