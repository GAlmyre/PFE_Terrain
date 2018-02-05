#ifndef TERRAINTINTIN_SCENE_H
#define TERRAINTINTIN_SCENE_H

#include <memory>

#include "OpenGL.h"
#include "FreeFlyCamera.h"
#include "Mesh.h"

class Scene {
public:
  Scene () : _funcs(nullptr), _camera(nullptr) {}

  virtual ~Scene() {}

  void setFuncs(GLFuncs *funcs) { _funcs = funcs; }
  void setCamera(FreeFlyCamera *camera) { _camera = camera; }

  virtual void initialize() = 0;
  virtual void render() = 0;
  virtual void update(float dt) = 0;
  virtual void resize(int width, int height) {
    _camera->setViewport(width, height);
    _funcs->glViewport( 0, 0, (GLint)width, (GLint)height );
  }
  virtual void clean() = 0;

protected:
  GLFuncs *_funcs;
  FreeFlyCamera *_camera;
};

using ScenePtr = std::shared_ptr<Scene>;

#endif //TERRAINTINTIN_SCENE_H
