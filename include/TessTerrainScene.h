#ifndef TERRAINTINTIN_TESSTERRAINSCENE_H
#define TERRAINTINTIN_TESSTERRAINSCENE_H

#include "Scene.h"
#include "Terrain.h"

class TessTerrainScene : public Scene {
public:
  virtual void initialize() {
    _terrain.init();
    _terrain.setHeightMap(QImage("../data/heightmaps/hm0_1024x1024.png"));

    _tessPrg = new QOpenGLShaderProgram();
    if (!_tessPrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/terrainTess.vert")) {
      qDebug() << _tessPrg->log();
    }
    if (!_tessPrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/terrainTess.frag")) {
      qDebug() << _tessPrg->log();
    }
    if (!_tessPrg->addShaderFromSourceFile(QOpenGLShader::TessellationControl, "../data/shaders/terrainTess.tesc")) {
      qDebug() << _tessPrg->log();
    }
    if (!_tessPrg->addShaderFromSourceFile(QOpenGLShader::TessellationEvaluation, "../data/shaders/terrainTess.tese")) {
      qDebug() << _tessPrg->log();
    }
    if (!_tessPrg->link()) {
      qDebug() << _tessPrg->log();
    }

    _innerTessLoc = _f->glGetProgramResourceLocation(_tessPrg->programId(), GL_UNIFORM, "TessLevelInner");
    _outerTessLoc = _f->glGetProgramResourceLocation(_tessPrg->programId(), GL_UNIFORM, "TessLevelOuter");
    _heightScaleLoc = _f->glGetProgramResourceLocation(_tessPrg->programId(), GL_UNIFORM, "heightScale");
    _viewLoc = _f->glGetProgramResourceLocation(_tessPrg->programId(), GL_UNIFORM, "view");
    _projectionLoc = _f->glGetProgramResourceLocation(_tessPrg->programId(), GL_UNIFORM, "projection");

    _camera = std::make_shared<FreeFlyCamera>();
    _camera->setPosition(Eigen::Vector3f(0,30,-5));
    _camera->setDirection(Eigen::Vector3f(5,-2,5));
    _camera->setViewport(600, 400);

    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glPatchParameteri(GL_PATCH_VERTICES, 3);
  }

  virtual void render() {
    _f->glClearColor(0.2, 0.2, 0.2, 1.0);
    _f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

//    _f->glCullFace(GL_FRONT);
//    _f->glEnable(GL_CULL_FACE);
    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
//    _f->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

    _f->glUseProgram(_tessPrg->programId());

    _f->glUniform1f(_innerTessLoc, TessLevelInner);
    _f->glUniform3fv(_outerTessLoc, 1, TessLevelOuter);
    _f->glUniform1f(_heightScaleLoc, _heightScale);
    _f->glUniformMatrix4fv(_viewLoc, 1, GL_FALSE, _camera->viewMatrix().data());
    _f->glUniformMatrix4fv(_projectionLoc, 1, GL_FALSE, _camera->projectionMatrix().data());

    _terrain.drawHardwareTessellation(*_tessPrg);
  }

  virtual void update(float dt) {
    _camera->update(dt);
  }

  virtual void clean() {
    _terrain.clean();
    delete _tessPrg;
  }

  virtual void resize(int width, int height) {
    _camera->setViewport(width, height);
    _f->glViewport( 0, 0, (GLint)width, (GLint)height );
  }

  virtual void mouseMoveEvent(QMouseEvent *e) {
    if (e->buttons() == Qt::LeftButton)
      _camera->processMouseMove(e->x(), e->y());
  }

  virtual void wheelEvent(QWheelEvent *e) {
  }

  virtual void mousePressEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
      _camera->processMousePress(e->x(), e->y());
  }

  virtual void mouseReleaseEvent(QMouseEvent *e) {
    if (e->button() == Qt::LeftButton)
      _camera->processMouseRelease();
  }

  virtual void keyPressEvent(QKeyEvent *e) {
    switch (e->key())
    {
      case Qt::Key_Up:
      case Qt::Key_Z:
        _camera->processKeyPress(FreeFlyCamera::KEY_FORWARD);
        break;
      case Qt::Key_Down:
      case Qt::Key_S:
        _camera->processKeyPress(FreeFlyCamera::KEY_BACKWARD);
        break;
      case Qt::Key_Right:
      case Qt::Key_D:
        _camera->processKeyPress(FreeFlyCamera::KEY_RIGHT);
        break;
      case Qt::Key_Left:
      case Qt::Key_Q:
        _camera->processKeyPress(FreeFlyCamera::KEY_LEFT);
        break;
      case Qt::Key_E:
        _camera->processKeyPress(FreeFlyCamera::KEY_UP);
        break;
      case Qt::Key_F:
        _camera->processKeyPress(FreeFlyCamera::KEY_DOWN);
        break;
      default:break;
    }
  }

  virtual void keyReleaseEvent(QKeyEvent *e) {
    switch (e->key())
    {
      case Qt::Key_Up:
      case Qt::Key_Z:
        _camera->processKeyRelease(FreeFlyCamera::KEY_FORWARD);
        break;
      case Qt::Key_Down:
      case Qt::Key_S:
        _camera->processKeyRelease(FreeFlyCamera::KEY_BACKWARD);
        break;
      case Qt::Key_Right:
      case Qt::Key_D:
        _camera->processKeyRelease(FreeFlyCamera::KEY_RIGHT);
        break;
      case Qt::Key_Left:
      case Qt::Key_Q:
        _camera->processKeyRelease(FreeFlyCamera::KEY_LEFT);
        break;
      case Qt::Key_E:
        _camera->processKeyRelease(FreeFlyCamera::KEY_UP);
        break;
      case Qt::Key_F:
        _camera->processKeyRelease(FreeFlyCamera::KEY_DOWN);
        break;
      default:break;
    }
  }

  virtual void focusOutEvent(QFocusEvent *event) {
    _camera->stopMovement();
  }

private:
  QOpenGLShaderProgram *_tessPrg = nullptr;
  Terrain _terrain;

  std::shared_ptr<FreeFlyCamera> _camera;

  GLint _viewLoc;
  GLint _projectionLoc;
  GLint _innerTessLoc;
  GLint _outerTessLoc;
  GLint _heightScaleLoc;

  float TessLevelInner = 10.f;
  float TessLevelOuter[3] = { 6.f, 6.f, 6.f };
  float _heightScale = 60.f;
};

#endif //TERRAINTINTIN_TESSTERRAINSCENE_H
