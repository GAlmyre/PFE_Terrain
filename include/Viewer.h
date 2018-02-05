//Viewer.hpp

#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <OpenGL.h>
#include <QtWidgets>

#include "Scene.h"
#include "FreeFlyCamera.h"

// This class will handle the rendering of the VEF it stores using openGL
class Viewer : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit Viewer(QWidget *parent = nullptr);
  ~Viewer();

protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void mouseMoveEvent(QMouseEvent *e);
  void wheelEvent(QWheelEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);
  void keyPressEvent(QKeyEvent *e);
  void keyReleaseEvent(QKeyEvent *e);
  void focusOutEvent(QFocusEvent *event);

protected slots:
  void messageLogged(const QOpenGLDebugMessage &msg);
  void updateScene();

signals:
  void fpsChanged(float fps);

private:
  void updateFPSCount();

private:
  GLFuncs *_funcs;

  QTime _time;
  float _fpsSum;
  float _previousTime;
  float _dt;
  unsigned long _frameNumber;

  QOpenGLDebugLogger *_debugLogger;

  FreeFlyCamera _camera;
  ScenePtr _scene;
};

#endif // VIEWER_HPP
