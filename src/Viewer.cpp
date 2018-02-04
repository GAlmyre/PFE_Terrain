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
  //_funcs->glEnable(GL_BLEND);
  //_funcs->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);

  //shader init
  _simplePrg = new QOpenGLShaderProgram();
  _simplePrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
  _simplePrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
  _simplePrg->link();
  _simplePrg->bind();
  _simplePrg->release();
  //_track = false;
  //_move = false;

  _camera = new FreeFlyCamera(Eigen::Vector3f(0,0,-2), Eigen::Vector3f(0,-1,0), 600, 400);
  _camera->setPerspective(70, 0.1, 10000);
  _mesh.createGrid(10.,10.,10,10, false);
}

void Viewer::paintGL(){

  _funcs->glClearColor(0.2, 0.2, 0.2, 1.0);
  _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  //fixed projection matrix for debug purpose
  QMatrix4x4 projMat;
  projMat.setToIdentity();
  projMat.perspective(90, 4./3,  0.1, 10000);

  QMatrix4x4 viewMat;
  viewMat.setToIdentity();
  viewMat.lookAt(QVector3D(0,2,0), QVector3D(3,0,3), QVector3D(0,1,0));

  
  _simplePrg->bind();
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), QMatrix4x4(_camera->projectionMatrix().data()).transposed());
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), QMatrix4x4(_camera->viewMatrix().data()).transposed());
  _funcs->glDepthFunc(GL_LESS);
  _simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), projMat); 
  _simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), viewMat);
  _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(1,0,0));
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("world_mat"), QMatrix4x4(_mesh.worldMatrix().data()).transposed());
  _mesh.draw(*_simplePrg);

  //_funcs->glDepthFunc(GL_LEQUAL);
  _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(0,1,0));
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  _mesh.draw(*_simplePrg);
  
  _simplePrg->release();
}

void Viewer::resizeGL(int width, int height){
  _camera->setViewport(width, height);
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
