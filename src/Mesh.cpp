#include <iostream>

#include "Mesh.h"

Mesh::Mesh()
  : _position(Eigen::Vector3f::Constant(0)), _updateWorldMat(true), _initialized(false)
{}

Mesh::~Mesh()
{
}

void Mesh::initVAO() {
  _vertexBuffer = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
  _vertexBuffer->create();
  _vertexBuffer->bind();
  _vertexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  _vertexBuffer->allocate(&(_vertices[0]), sizeof(Mesh::Vertex)*_vertices.size());
  _indexBuffer = new QOpenGLBuffer(QOpenGLBuffer::IndexBuffer);
  _indexBuffer->create();
  _indexBuffer->bind();
  _indexBuffer->setUsagePattern(QOpenGLBuffer::StaticDraw);
  _indexBuffer->allocate(&(_indices[0]), sizeof(unsigned int)*_indices.size());
  _vertexArray.create();
  _initialized = true;
  _indexBuffer->release();
  _vertexBuffer->release();
}


void Mesh::draw(QOpenGLShaderProgram &shader){

  if(!_initialized)
    initVAO();

  //Eigen::Matrix4f worldMat = this->worldMatrix();
  //shader.setUniformValue(shader.uniformLocation("world_mat"), QMatrix4x4(worldMat.data()).transposed());//*(worldMat.data()));
  QMatrix4x4 worldMat;
  worldMat.setToIdentity();
  shader.setUniformValue(shader.uniformLocation("world_mat"), worldMat);
  _vertexArray.bind();
  _vertexBuffer->bind();
  _indexBuffer->bind();

  int vertex_loc = shader.attributeLocation("vtx_position");
  if(vertex_loc>=0) {
    shader.setAttributeBuffer(vertex_loc, GL_FLOAT, offsetof(Mesh::Vertex,position), 3, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(vertex_loc);
  }

  int normal_loc = shader.attributeLocation("vtx_normal");
  if(normal_loc>=0) {
    shader.setAttributeBuffer(normal_loc, GL_FLOAT, offsetof(Mesh::Vertex,normal), 3, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(normal_loc);
  }

  int texcoord_loc = shader.attributeLocation("vtx_texcoord");
  if(texcoord_loc>=0) {
    shader.setAttributeBuffer(texcoord_loc, GL_FLOAT, offsetof(Mesh::Vertex,texcoord), 2, sizeof(Mesh::Vertex));
    shader.enableAttributeArray(texcoord_loc);
  }

  glDrawElements(GL_TRIANGLES, _indices.size(), GL_UNSIGNED_INT, 0);

  if(vertex_loc)
    shader.disableAttributeArray(vertex_loc);
  if(normal_loc)
    shader.disableAttributeArray(normal_loc);
  if(texcoord_loc)
    shader.disableAttributeArray(texcoord_loc);
    
  _indexBuffer->release();
  _vertexBuffer->release();
  _vertexArray.release();
}

void Mesh::clear()
{
  _updateWorldMat = true;
  _position = Eigen::Vector3f::Constant(0);

  _initialized = false;

  _vertices.clear();
  _indices.clear();
}

void Mesh::clean() {
  if (_vertexBuffer) {
    _vertexBuffer->destroy();
    delete _vertexBuffer;
  }
  if (_indexBuffer) {
    _indexBuffer->destroy();
    delete _indexBuffer;
  }

  _vertexArray.destroy();
}

Eigen::Matrix4f Mesh::worldMatrix()
{
  //TODO

}


