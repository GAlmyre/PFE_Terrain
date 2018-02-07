#ifndef TERRAINTINTIN_TERRAINSCENE_H
#define TERRAINTINTIN_TERRAINSCENE_H

#include "Scene.h"
#include "Mesh.h"

class TerrainScene : public Scene {
public:
  void initialize() override {
    //shader init
    _simplePrg = new QOpenGLShaderProgram();
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
    _simplePrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
    _simplePrg->link();

    _camera->setPosition(Eigen::Vector3f(0,10,-5));
    _camera->setDirection(-Eigen::Vector3f(0,10,-5));
    _camera->setViewport(600, 400);

    _mesh.createGrid(10,10,10,20, false);

    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glEnable(GL_BLEND);
    _f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  }

  void render() override {
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

    _simplePrg->bind();
    //_simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), QMatrix4x4(_camera->projectionMatrix().data()).transposed());
    //_simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), QMatrix4x4(_camera->viewMatrix().data()).transposed());
    _f->glDepthFunc(GL_LESS);
//  _simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), projMat);
    //_simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), viewMat);

    _f->glUniformMatrix4fv(_simplePrg->uniformLocation("view_mat"), 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_simplePrg->uniformLocation("proj_mat"), 1, GL_FALSE, _camera->projectionMatrix().data());

    _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(1,0,0));
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    //_simplePrg->setUniformValue(_simplePrg->uniformLocation("world_mat"), QMatrix4x4(_mesh.worldMatrix().data()).transposed());
    _mesh.draw(*_simplePrg);

    _f->glDepthFunc(GL_LEQUAL);
    _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(0,1,0));
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
    _mesh.draw(*_simplePrg);

    _simplePrg->release();
  }

  void update(float dt) override {
    _camera->update(dt);
  }

  void clean() override {
    delete _simplePrg;
    _mesh.clean();
  }

private:
  QOpenGLShaderProgram * _simplePrg;
  Mesh _mesh;
};

#endif //TERRAINTINTIN_TERRAINSCENE_H
