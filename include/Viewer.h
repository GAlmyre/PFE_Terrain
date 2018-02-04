//Viewer.hpp

#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <OpenGL.h>
#include <QtWidgets>

// This class will handle the rendering of the VEF it stores using openGL
class Viewer : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit Viewer(QWidget *parent = nullptr);
  ~Viewer();

  void eventFromParent(QKeyEvent *e);
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

protected slots:
  void messageLogged(const QOpenGLDebugMessage &msg);

signals:
  void fpsChanged(float fps);

private:
  void updateFPSCount(float t);

private:
  QOpenGLShaderProgram *_shader;

  QOpenGLVertexArrayObject _object;
  QOpenGLBuffer _vertexBufferId;
  QOpenGLBuffer* _faceBuffer;
  GLFuncs *_funcs;

  QTime _time;
  float _fpsSum;
  float _previousTime;
  unsigned long _frameNumber;

  QOpenGLDebugLogger *_debugLogger;
};

#endif // VIEWER_HPP
