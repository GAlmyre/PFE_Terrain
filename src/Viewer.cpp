#include "Viewer.h"

#include <iostream>

Viewer::Viewer(QWidget *parent)
  : QOpenGLWidget(parent)
{
  QSurfaceFormat format;
  format.setVersion(4, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setOption(QSurfaceFormat::DebugContext);

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

void Viewer::initializeGL() {
  // Load functions
  _funcs = QOpenGLContext::currentContext()->versionFunctions<GLFuncs>();

  // Print OpenGL information
  std::cout << "Loaded OpenGL" << std::endl
            << "Version : " << _funcs->glGetString(GL_VERSION) << std::endl
            << "Shading Version : " << _funcs->glGetString(GL_SHADING_LANGUAGE_VERSION) << std::endl;

  // Initialize synchronous debugger
  _debugLogger = new QOpenGLDebugLogger(this);
  connect(_debugLogger, &QOpenGLDebugLogger::messageLogged, this, &Viewer::messageLogged);
  if (_debugLogger->initialize()) {
    _debugLogger->startLogging(QOpenGLDebugLogger::SynchronousLogging);
    _debugLogger->enableMessages();
  }

  _funcs->glClearColor(0.6, 0.2, 0.2, 1.0);
  //f->glEnable(GL_CULL_FACE);
  _funcs->glEnable(GL_BLEND);
  _funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

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

  _funcs->glClearColor(0.2, 0.2, 0.2, 1.0);
  _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);
}

void Viewer::resizeGL(int width, int height){

  _funcs->glViewport( 0, 0, (GLint)width, (GLint)height );
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

void Viewer::messageLogged(const QOpenGLDebugMessage &msg) {
  if(msg.id() == 131169 || msg.id() == 131185 || msg.id() == 131218 || msg.id() == 131204) return;
  qDebug() << msg;
}
