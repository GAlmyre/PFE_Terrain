#include <iostream>

#include "Mesh.h"

Mesh::Mesh()
  : _position(Eigen::Vector3f::Constant(0)), _updateWorldMat(true), _initialized(false)
{}

Mesh::~Mesh()
{
  if(_vertexBuffer != NULL){
    _vertexBuffer->destroy();
    delete _vertexBuffer;
  }
  if(_indexBuffer != NULL){
    _indexBuffer->destroy();
    delete _indexBuffer;
  }

  _vertexArray.destroy();

}

void Mesh::draw(const QOpenGLShaderProgram &shader){

  if(!_initialized)
    initVAO();

    /*
  shader->setUniformValue(shader->uniformLocation("world_mat"), this->worldMatrix());
  _vertexArray.bind();
  _vertexBuffer->bind();
  _indexBuffer->bind();

  int vertex_loc = shader->attributeLocation("vtx_position");
  if(vertex_loc>=0) {
    shader->setAttributeBuffer(vertex_loc, GL_FLOAT, offsetof(VEF::Vertex,position), 3, sizeof(VEF::Vertex));
    shader->enableAttributeArray(vertex_loc);
  }

  int normal_loc = shader->attributeLocation("vtx_normal");
  if(normal_loc>=0) {
    //TODO j'en étais là !
    shader->setAttributeBuffer(vertex_loc, GL_FLOAT, offsetof(VEF::Vertex,position), 3, sizeof(VEF::Vertex));
    shader->enableAttributeArray(vertex_loc);
  }

  glDrawElements(GL_TRIANGLES, _faces.size(), GL_UNSIGNED_INT, 0);

  if(vertex_loc)
    shader->disableAttributeArray(vertex_loc);
  
  _indexBuffer->release();
  _vertexBuffer->release();
  _vertexArray.release();
    */
}

void Mesh::createGrid(float width, float height, int nbRows, int nbColumns, bool quads)
{
  //1st step : creating all the vertices
  for(int i=0; i <= nbRows+1; ++i)
    for(int j=0; j <= nbColumns+1; ++j){
      float normPos_i = (float)i/nbRows, normPos_j = (float)j/nbColumns;
      _vertices.push_back(Vertex(Eigen::Vector3f(normPos_i*width, 0, normPos_j*height),
				 Eigen::Vector3f(0, 1, 0),
				 Eigen::Vector2f(normPos_i,normPos_j)));
    }
  //2nd step : creating all the faces
  int nbCells = nbRows*nbColumns;
  int lineOffset = -1;
  for(int i=0; i < nbCells+1; ++i){
    if(i%nbRows==0)
      lineOffset++;
    int p1, p2, p3, p4;
    /* we get p1 p2 p3 p4 the indices of the vertices of the ith quad patch 
       p1 --- p2
       |      |
       |      |
       p3 --- p4
     */
    p1 = i+lineOffset;
    p2 = p1+1;
    p3 = p1+nbColumns+1;
    p4 = p3+1;
    if(quads){
      _indices.push_back((unsigned int)p1);
      _indices.push_back((unsigned int)p3);
      _indices.push_back((unsigned int)p4);
      _indices.push_back((unsigned int)p2);
    }
    else{
      //split the quad into two triangles
      _indices.push_back((unsigned int)p1);
      _indices.push_back((unsigned int)p3);
      _indices.push_back((unsigned int)p2);
      _indices.push_back((unsigned int)p2);
      _indices.push_back((unsigned int)p3);
      _indices.push_back((unsigned int)p4);
    }
  }  
}

void Mesh::clear()
{
  _updateWorldMat = true;
  _position = Eigen::Vector3f::Constant(0);

  _initialized = false;

  _vertices.clear();
  _indices.clear();
}

Eigen::Matrix4f Mesh::worldMatrix()
{
  //TODO

}


