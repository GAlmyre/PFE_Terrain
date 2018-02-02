#include "Mesh.h"

Mesh::Mesh()
  : _position(Eigen::Vector3f::Constant(0)), _updateWorldMat(false), _initialized(false)
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


Eigen::Matrix4f Mesh::worldMatrix()
{

}


