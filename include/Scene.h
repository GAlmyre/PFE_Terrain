#ifndef TERRAINTINTIN_SCENE_H
#define TERRAINTINTIN_SCENE_H

#include <memory>
#include <QDockWidget>

#include "OpenGL.h"
#include "FreeFlyCamera.h"

class Scene {
public:
  Scene () : _f(nullptr), _camera(nullptr), _dock(nullptr) {}
  virtual ~Scene() { if (_dock) delete _dock; }

  void setFuncs(GLFuncs *funcs) { _f = funcs; }
  void setCamera(FreeFlyCamera *camera) { _camera = camera; }
  QDockWidget *getDock() { _dock = createDock(); return _dock; };

  virtual void initialize() = 0;
  virtual void render() = 0;
  virtual void update(float dt) = 0;
  virtual void resize(int width, int height) {
    _camera->setViewport(width, height);
    _f->glViewport( 0, 0, (GLint)width, (GLint)height );
  }
  virtual void clean() = 0;

  virtual QDockWidget *createDock() { _dock = nullptr; };
protected:
  GLFuncs *_f;
  FreeFlyCamera *_camera;
private:
  QDockWidget *_dock = nullptr;
};

using ScenePtr = std::shared_ptr<Scene>;

#endif //TERRAINTINTIN_SCENE_H
