//Viewer.hpp

#ifndef VIEWER_HPP
#define VIEWER_HPP

#include <QtWidgets>
#include <QtOpenGL>
#include <QOpenGLShaderProgram>

// This class will handle the rendering of the VEF it stores using openGL
class Viewer : public QOpenGLWidget
{
  Q_OBJECT

public:
  explicit Viewer(QWidget *parent = 0);
  ~Viewer();

  void eventFromParent(QKeyEvent *e);
protected:
  void initializeGL();
  void paintGL();
  void resizeGL(int width, int height);

  QSize minimumSizeHint() const;
  QSize sizeHint() const;

  void timerEvent(QTimerEvent *);
  void mouseMoveEvent(QMouseEvent *e);
  void wheelEvent(QWheelEvent *e);
  void mousePressEvent(QMouseEvent *e);
  void mouseReleaseEvent(QMouseEvent *e);

private:
  QOpenGLShaderProgram * _shader;

  QOpenGLVertexArrayObject _object;
  QOpenGLBuffer _vertexBufferId;
  QOpenGLBuffer* _faceBuffer;

  bool _track;
  bool _move;
  QVector2D _prevPos;
  float _angularSpeed;
  QVector3D _rotationAxis;

  QBasicTimer _timer;
};

#endif // VIEWER_HPP
