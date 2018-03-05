#ifndef TERRAINTINTIN_SPHERE_H
#define TERRAINTINTIN_SPHERE_H

#include <Eigen/Geometry>
#include <QOpenGLShaderProgram>
#include "OpenGL.h"

class Point
{
public:
  static void draw(GLFuncs *f, QOpenGLShaderProgram* prg, const Eigen::Vector3f& p)
  {
    unsigned int vertexBufferId;
    f->glGenBuffers(1,&vertexBufferId);
    f->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f), p.data(), GL_STATIC_DRAW);

    unsigned int vertexArrayId;
    f->glGenVertexArrays(1,&vertexArrayId);

    // bind the vertex array
    f->glBindVertexArray(vertexArrayId);

    f->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    int vertex_loc = prg->attributeLocation("vtx_position");
    f->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    f->glEnableVertexAttribArray(vertex_loc);

    glPointSize(10.f);
    glDrawArrays(GL_POINTS,0,1);

    f->glDisableVertexAttribArray(vertex_loc);
    f->glBindVertexArray(0);

    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glDeleteBuffers(1, &vertexBufferId);
    f->glBindVertexArray(0);
    f->glDeleteVertexArrays(1, &vertexArrayId);
  }
};

class Line
{
public:
  static void draw(GLFuncs *f, QOpenGLShaderProgram* prg, const Eigen::Vector3f& p1, const Eigen::Vector3f& p2)
  {
    Eigen::Vector3f mPoints[2] = {p1, p2};

    unsigned int vertexBufferId;
    f->glGenBuffers(1,&vertexBufferId);
    f->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);
    f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f)*2, mPoints[0].data(), GL_STATIC_DRAW);

    unsigned int vertexArrayId;
    f->glGenVertexArrays(1,&vertexArrayId);

    // bind the vertex array
    f->glBindVertexArray(vertexArrayId);

    f->glBindBuffer(GL_ARRAY_BUFFER, vertexBufferId);

    int vertex_loc = prg->attributeLocation("vtx_position");
    f->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
    f->glEnableVertexAttribArray(vertex_loc);

    glDrawArrays(GL_LINES,0,2);

    f->glDisableVertexAttribArray(vertex_loc);
    f->glBindVertexArray(0);

    f->glBindBuffer(GL_ARRAY_BUFFER, 0);
    f->glDeleteBuffers(1, &vertexBufferId);
    f->glBindVertexArray(0);
    f->glDeleteVertexArrays(1, &vertexArrayId);
  }
};


class Sphere {
public:
  Sphere() : _radius(0), _f(nullptr) {}

  ~Sphere() {
    if (_f) {
      _f->glDeleteVertexArrays(1, &_vao);
      _f->glDeleteBuffers(6, _vbo);
    }
  }

  void generate(float radius=1.f, int nU=40, int nV=40) {
    _radius = radius;

    int nVertices  = (nU + 1) * (nV + 1);
    int nTriangles =  nU * nV * 2;

    _vertices.resize(nVertices);
    _normals.resize(nVertices);
    _tangents.resize(nVertices);
    _texCoords.resize(nVertices);
    _colors.resize(nVertices);
    _indices.resize(3*nTriangles);

    for(int v=0;v<=nV;++v)
    {
      for(int u=0;u<=nU;++u)
      {

        float theta = u / float(nU) * M_PI;
        float phi 	= v / float(nV) * M_PI * 2;

        int index 	= u +(nU+1)*v;

        Eigen::Vector3f vertex, tangent, normal;
        Eigen::Vector2f texCoord;

        // normal
        normal[0] = sin(theta) * cos(phi);
        normal[1] = sin(theta) * sin(phi);
        normal[2] = cos(theta);
        normal.normalize();

        // position
        vertex = normal * _radius;

        // tangent
        theta += M_PI_2;
        tangent[0] = sin(theta) * cos(phi);
        tangent[1] = sin(theta) * sin(phi);
        tangent[2] = cos(theta);
        tangent.normalize();

        // texture coordinates
        texCoord[1] = u / float(nU);
        texCoord[0] = v / float(nV);

        _vertices[index] = vertex;
        _normals[index] = normal;
        _tangents[index] = tangent;
        _texCoords [index] = texCoord;
        _colors[index] = Eigen::Vector3f(0.6f,0.2f,0.8f);
        _bbox.extend(vertex);
      }
    }

    int index = 0;
    for(int v=0;v<nV;++v)
    {
      for(int u=0;u<nU;++u)
      {
        int vindex 	= u + (nU+1)*v;

        _indices[index+0] = vindex;
        _indices[index+1] = vindex+1 ;
        _indices[index+2] = vindex+1 + (nU+1);

        _indices[index+3] = vindex;
        _indices[index+4] = vindex+1 + (nU+1);
        _indices[index+5] = vindex   + (nU+1);

        index += 6;
      }
    }
  }

  void init(GLFuncs *funcs) {
    bool genBuffers = (_f == nullptr);
    _f = funcs;

    if (genBuffers) {
      _f->glGenVertexArrays(1, &_vao);
      _f->glGenBuffers(6, _vbo);
    }

    _f->glBindVertexArray(_vao);

    _f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);
    _f->glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * _indices.size(), _indices.data(),  GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * _vertices.size(), _vertices.data(), GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * _colors.size(), _colors.data(), GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * _normals.size(), _normals.data(), GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector3f) * _tangents.size(), _tangents.data(), GL_STATIC_DRAW);

    _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[5]);
    _f->glBufferData(GL_ARRAY_BUFFER, sizeof(Eigen::Vector2f) * _texCoords.size(), _texCoords.data(), GL_STATIC_DRAW);

    _f->glBindVertexArray(0);
  }

  void display(QOpenGLShaderProgram &shader) {

    _f->glBindVertexArray(_vao);

    _f->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, _vbo[0]);

    int vertex_loc = shader.attributeLocation("vtx_position");
    _f->glEnableVertexAttribArray(vertex_loc);
    if(vertex_loc>=0){
      _f->glEnableVertexAttribArray(vertex_loc);
      _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[1]);
      _f->glVertexAttribPointer(vertex_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int color_loc = shader.attributeLocation("vtx_color");
    if(color_loc>=0){
      _f->glEnableVertexAttribArray(color_loc);
      _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[2]);
      _f->glVertexAttribPointer(color_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int normal_loc = shader.attributeLocation("vtx_normal");
    if(normal_loc>=0){
      _f->glEnableVertexAttribArray(normal_loc);
      _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[3]);
      _f->glVertexAttribPointer(normal_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int tangent_loc = shader.attributeLocation("vtx_tangent");
    if(tangent_loc>=0){
      _f->glEnableVertexAttribArray(tangent_loc);
      _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[4]);
      _f->glVertexAttribPointer(tangent_loc, 3, GL_FLOAT, GL_FALSE, 0, 0);
    }

    int texCoord_loc = shader.attributeLocation("vtx_texcoord");
    if(texCoord_loc>=0){
      _f->glEnableVertexAttribArray(texCoord_loc);
      _f->glBindBuffer(GL_ARRAY_BUFFER, _vbo[5]);
      _f->glVertexAttribPointer(texCoord_loc, 2, GL_FLOAT, GL_FALSE, 0, 0);
    }

    glDrawElements(GL_TRIANGLES, _indices.size(),  GL_UNSIGNED_INT, 0);

    if(vertex_loc>=0)
      _f->glDisableVertexAttribArray(vertex_loc);
    if(color_loc>=0)
      _f->glDisableVertexAttribArray(color_loc);
    if(normal_loc>=0)
      _f->glDisableVertexAttribArray(normal_loc);
    if(tangent_loc>=0)
      _f->glDisableVertexAttribArray(tangent_loc);
    if(texCoord_loc>=0)
      _f->glDisableVertexAttribArray(texCoord_loc);


    _f->glBindVertexArray(0);
  }

  float radius() const { return _radius; }

public:
  Eigen::Affine3f _transformation;

private:
  GLFuncs *_f;

  GLuint _vao;
  GLuint _vbo[6];

  std::vector<int>              _indices;   /** vertex indices */
  std::vector<Eigen::Vector3f>	_vertices;  /** 3D positions */
  std::vector<Eigen::Vector3f>	_colors;    /** colors */
  std::vector<Eigen::Vector3f>	_normals;   /** 3D normals */
  std::vector<Eigen::Vector3f>	_tangents;  /** 3D tangent to surface */
  std::vector<Eigen::Vector2f>	_texCoords; /** 2D texture coordinates */

  Eigen::AlignedBox3f _bbox;

  float _radius;
};

#endif //TERRAINTINTIN_SPHERE_H
