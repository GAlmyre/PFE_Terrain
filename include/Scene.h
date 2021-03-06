#ifndef TERRAINTINTIN_SCENE_H
#define TERRAINTINTIN_SCENE_H

#include <memory>
#include <QDockWidget>

#include "OpenGL.h"
#include "FreeFlyCamera.h"
#include "MainWindow.h"

class Scene {
public:
  Scene () : _f(nullptr),  _dock(nullptr) {}
  virtual ~Scene() { if (_dock) delete _dock; }

  void setFuncs(GLFuncs *funcs) { _f = funcs; }
  QDockWidget *getDock() { _dock = createDock(); return _dock; };

  virtual void initialize() = 0;
  virtual void render() = 0;
  virtual void update(float dt) = 0;
  virtual void resize(int width, int height) {
    _f->glViewport( 0, 0, (GLint)width, (GLint)height );
  }
  virtual void clean() = 0;

  virtual QDockWidget *createDock() { return nullptr; };
  virtual void connectToMainWindow(MainWindow *mw) { };

  virtual void mouseMoveEvent(QMouseEvent *e) {};
  virtual void wheelEvent(QWheelEvent *e) {};
  virtual void mousePressEvent(QMouseEvent *e) {};
  virtual void mouseReleaseEvent(QMouseEvent *e) {};
  virtual void keyPressEvent(QKeyEvent *e) {};
  virtual void keyReleaseEvent(QKeyEvent *e) {};
  virtual void focusOutEvent(QFocusEvent *event) {};
protected:
  GLFuncs *_f;
private:
  QDockWidget *_dock;
};

using ScenePtr = std::shared_ptr<Scene>;

#endif //TERRAINTINTIN_SCENE_H
