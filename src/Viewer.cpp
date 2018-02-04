#include "Viewer.h"

#include <iostream>

Viewer::Viewer(QWidget *parent)
  : QOpenGLWidget(parent), _frameNumber(0), _fpsSum(0), _dt(0)
{
  // Set OpenGL format
  QSurfaceFormat format;
  format.setVersion(3, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setOption(QSurfaceFormat::DebugContext);
  setFormat(format);

  // Auto update opengl drawing
  QTimer* timer = new QTimer( this );
  connect(timer, &QTimer::timeout, this, &Viewer::updateScene);
  timer->start(5);
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

  //shader init
  _simplePrg = new QOpenGLShaderProgram();
  _simplePrg->addShaderFromSourceFile(QOpenGLShader::Vertex, "../data/shaders/simple.vert");
  _simplePrg->addShaderFromSourceFile(QOpenGLShader::Fragment, "../data/shaders/simple.frag");
  _simplePrg->link();

  _camera.setPosition(Eigen::Vector3f(0,10,-5));
  _camera.setDirection(Eigen::Vector3f(5,0,10) - Eigen::Vector3f(0,10,-5));
  _camera.setViewport(600, 400);
  //_camera.setPerspective(70, 0.1, 10000);

  _mesh.createGrid(10,10,10,20, false);
}

void Viewer::paintGL(){
  _funcs->glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT);

  //fixed projection matrix for debug purpose
  QMatrix4x4 projMat;
  projMat.setToIdentity();
  projMat.perspective(90, 4./3,  0.1, 10000);

  QMatrix4x4 viewMat;
  viewMat.setToIdentity();
  viewMat.lookAt(QVector3D(0,10,-5), QVector3D(5,0,10), QVector3D(0,1,0));



  _simplePrg->bind();
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), QMatrix4x4(_camera.projectionMatrix().data()).transposed());
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), QMatrix4x4(_camera.viewMatrix().data()).transposed());
  _funcs->glDepthFunc(GL_LESS);
//  _simplePrg->setUniformValue(_simplePrg->uniformLocation("proj_mat"), projMat);
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("view_mat"), viewMat);

  _funcs->glUniformMatrix4fv(_simplePrg->uniformLocation("view_mat"), 1, GL_FALSE, _camera.viewMatrix().data());
  _funcs->glUniformMatrix4fv(_simplePrg->uniformLocation("proj_mat"), 1, GL_FALSE, _camera.projectionMatrix().data());

  _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(1,0,0));
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
  //_simplePrg->setUniformValue(_simplePrg->uniformLocation("world_mat"), QMatrix4x4(_mesh.worldMatrix().data()).transposed());
  _mesh.draw(*_simplePrg);

  _funcs->glDepthFunc(GL_LEQUAL);
  _simplePrg->setUniformValue(_simplePrg->uniformLocation("v_color"), QVector3D(0,1,0));
  _funcs->glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  _mesh.draw(*_simplePrg);

  _simplePrg->release();


  _frameNumber++;
}

void Viewer::resizeGL(int width, int height){
  _camera.setViewport(width, height);
  _funcs->glViewport( 0, 0, (GLint)width, (GLint)height );
}

void Viewer::updateScene() {
  // Update time (in ms)
  float time = _time.elapsed();
  _dt = time - _previousTime;
  _previousTime = time;
  updateFPSCount();

  _camera.update(_dt);
  update();
}

void Viewer::mousePressEvent(QMouseEvent *e){
  setFocus();
  if (e->button() == Qt::LeftButton)
    _camera.processMousePress(e->x(), e->y());
}

void Viewer::mouseReleaseEvent(QMouseEvent *e){
  if (e->button() == Qt::LeftButton)
    _camera.processMouseRelease();
}

void Viewer::mouseMoveEvent(QMouseEvent *e){
  if (e->buttons() == Qt::LeftButton)
    _camera.processMouseMove(e->x(), e->y());
}

void Viewer::wheelEvent(QWheelEvent *e){

}

void Viewer::keyPressEvent(QKeyEvent *e) {
  switch (e->key())
  {
    case Qt::Key_Escape:
      QCoreApplication::instance()->quit();
      break;
      //Wireframe
    case Qt::Key_W:
//      if (e->modifiers() & Qt::ControlModifier) {
//        m_wireframe = !m_wireframe;
//        makeCurrent();
//        m_engine->setWireframe(m_wireframe);
//        doneCurrent();
//      }
      break;
      //Reload Shaders
    case Qt::Key_R:
//      if (e->modifiers() & Qt::ControlModifier) {
//        m_engine->reloadShaders();
//      }
      break;
      //Movement
    case Qt::Key_Up:
    case Qt::Key_Z:
      _camera.processKeyPress(FreeFlyCamera::KEY_FORWARD);
      break;
    case Qt::Key_Down:
    case Qt::Key_S:
      _camera.processKeyPress(FreeFlyCamera::KEY_BACKWARD);
      break;
    case Qt::Key_Right:
    case Qt::Key_D:
      _camera.processKeyPress(FreeFlyCamera::KEY_RIGHT);
      break;
    case Qt::Key_Left:
    case Qt::Key_Q:
      _camera.processKeyPress(FreeFlyCamera::KEY_LEFT);
      break;
    case Qt::Key_E:
      _camera.processKeyPress(FreeFlyCamera::KEY_UP);
      break;
    case Qt::Key_F:
      _camera.processKeyPress(FreeFlyCamera::KEY_DOWN);
      break;
    default:
      QOpenGLWidget::keyPressEvent(e);
  }
}

void Viewer::keyReleaseEvent(QKeyEvent *e) {
  switch (e->key())
  {
    case Qt::Key_Up:
    case Qt::Key_Z:
      _camera.processKeyRelease(FreeFlyCamera::KEY_FORWARD);
      break;
    case Qt::Key_Down:
    case Qt::Key_S:
      _camera.processKeyRelease(FreeFlyCamera::KEY_BACKWARD);
      break;
    case Qt::Key_Right:
    case Qt::Key_D:
      _camera.processKeyRelease(FreeFlyCamera::KEY_RIGHT);
      break;
    case Qt::Key_Left:
    case Qt::Key_Q:
      _camera.processKeyRelease(FreeFlyCamera::KEY_LEFT);
      break;
    case Qt::Key_E:
      _camera.processKeyRelease(FreeFlyCamera::KEY_UP);
      break;
    case Qt::Key_F:
      _camera.processKeyRelease(FreeFlyCamera::KEY_DOWN);
      break;
      //case Qt::Key_F11:
      //	isFullScreen() ? showNormal() : showFullScreen();
      //	break;
    default:
      QOpenGLWidget::keyReleaseEvent(e);
  }
}

void Viewer::focusOutEvent(QFocusEvent * event) {
  _camera.stopMovement();
}

void Viewer::messageLogged(const QOpenGLDebugMessage &msg) {
  if(msg.id() == 131169 || msg.id() == 131185 || msg.id() == 131218 || msg.id() == 131204) return;
  qDebug() << msg;
}

void Viewer::updateFPSCount() {
  unsigned long n = _frameNumber % 50;
  if (n == 0) {
    emit fpsChanged(_fpsSum / 50.f);
    _fpsSum = 0.f;
  }
  _fpsSum += 1000.f / _dt;
}


