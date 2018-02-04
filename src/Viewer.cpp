#include "Viewer.h"

#include <iostream>

Viewer::Viewer(QWidget *parent)
  : QOpenGLWidget(parent), _frameNumber(0), _fpsSum(0)
{
  // Set OpenGL format
  QSurfaceFormat format;
  format.setVersion(4, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setOption(QSurfaceFormat::DebugContext);
  setFormat(format);

  // Auto update opengl drawing
  QTimer* timer = new QTimer( this );
  connect(timer, SIGNAL(timeout()), this, SLOT(update()));
  timer->start(0);
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

  _funcs->glClearColor(0.2, 0.2, 0.2, 1.0);
  //f->glEnable(GL_CULL_FACE);
  _funcs->glEnable(GL_BLEND);
  _funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  // Start timer
  _time.start();
  _previousTime = _time.elapsed();

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
  // Update time (in ms)
  float time = _time.elapsed();
  float t = time - _previousTime;
  _previousTime = time;
  updateFPSCount(t);

  _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  _frameNumber++;
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

//Events received from MainWindow
void Viewer::eventFromParent(QKeyEvent *e){

  QWidget::keyPressEvent(e);

  update();
}

void Viewer::messageLogged(const QOpenGLDebugMessage &msg) {
  if(msg.id() == 131169 || msg.id() == 131185 || msg.id() == 131218 || msg.id() == 131204) return;
  qDebug() << msg;
}

void Viewer::updateFPSCount(float t) {
  unsigned long n = _frameNumber % 50;
  if (n == 0) {
    emit fpsChanged(_fpsSum / 50.f);
    _fpsSum = 0.f;
  }
  _fpsSum += 1000.f / t;
}
