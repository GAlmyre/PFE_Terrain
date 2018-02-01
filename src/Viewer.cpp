#include "Viewer.hpp"

#include <iostream>
#include <QOpenGLFunctions>
#include <QOpenGLContext>

Viewer::Viewer(QWidget *parent)
  : QOpenGLWidget(parent)
{
  QSurfaceFormat format;
  format.setVersion(3, 3);

  this->setFormat(format);

  _track = false;
  _move = false;
  _prevPos = QVector2D(0, 0);
  _timer.start(0, this);
}

Viewer::~Viewer(){
  if(!_shader)
    delete _shader;
}

QSize Viewer::minimumSizeHint() const{
  return QSize(50, 50);
}

QSize Viewer::sizeHint() const{
  return QSize(400, 400);
}

void Viewer::initializeGL(){

  QOpenGLFunctions * f = QOpenGLContext::currentContext()->functions();

  f->glClearColor(0.6, 0.2, 0.2, 1.0);
  //f->glEnable(GL_CULL_FACE);
  f->glEnable(GL_BLEND);
  f->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

/*  //shader init
  _shader = new QOpenGLShaderProgram();
  _shader->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
  _shader->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
  _shader->link();
  _shader->bind();
  _shader->release();
  _track = false;
  _move = false;*/
}

void Viewer::paintGL(){

  QOpenGLFunctions * f = QOpenGLContext::currentContext()->functions();
  f->glClearColor(0.2, 0.2, 0.2, 1.0);
  f->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void Viewer::resizeGL(int width, int height){
  QOpenGLFunctions * f = QOpenGLContext::currentContext()->functions();
  f->glViewport( 0, 0, (GLint)width, (GLint)height );
}

void Viewer::mousePressEvent(QMouseEvent *e){

}

void Viewer::mouseReleaseEvent(QMouseEvent *e){

}

void Viewer::mouseMoveEvent(QMouseEvent *e){

}

void Viewer::wheelEvent(QWheelEvent *e){

}

void Viewer::timerEvent(QTimerEvent *){
  update();
}

//Events received from MainWindow
void Viewer::eventFromParent(QKeyEvent *e){

  QWidget::keyPressEvent(e);

  update();
}
