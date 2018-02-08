#ifndef MESH_H
#define MESH_H

#include <vector>
#include <QtOpenGL>

#include "Eigen/Eigen"

class Mesh
{
 protected:
  struct Vertex
  {
    Vertex()
    : position(Eigen::Vector3f::Zero()), normal(Eigen::Vector3f::Zero()),
        texcoord(Eigen::Vector2f::Zero())
      {}
    Vertex(const Eigen::Vector3f& pos, const Eigen::Vector3f& n, const Eigen::Vector2f& tex)
    : position(pos), normal(n), texcoord(tex)
      {}
  
    Eigen::Vector3f position;
    Eigen::Vector3f normal;
    Eigen::Vector2f texcoord;
  };
  
 public:
  Mesh();
  ~Mesh();

  virtual void draw(QOpenGLShaderProgram &shader);

  void clear();

  virtual void clean();

  Eigen::Matrix4f worldMatrix();
 protected:
  std::vector<Vertex> _vertices;
  std::vector<unsigned int> _indices;

  void initVAO();
  
  bool _initialized;//true if VAO has been initialized
  QOpenGLVertexArrayObject _vertexArray;
  QOpenGLBuffer* _vertexBuffer;
  QOpenGLBuffer* _indexBuffer;
  
  bool _updateWorldMat;//true if _worldMat needs to be updated
  Eigen::Matrix4f _worldMat;
  Eigen::Vector3f _position;
};

#endif // MESH_H
