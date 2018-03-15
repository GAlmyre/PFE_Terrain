#include "Viewer.h"

#include <iostream>

#include "TerrainScene.h"
#include "TessTestScene.h"
#include "TessTerrainScene.h"
#include "TestInstanciationScene.h"
#include "PatchTessTestScene.h"

Viewer::Viewer(QMainWindow *parent)
  : QOpenGLWidget(parent), _frameNumber(0), _fpsSum(0), _dt(0), _mainWindow(parent)
{
  // Set OpenGL format
  QSurfaceFormat format;
  format.setVersion(4, 3);
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setOption(QSurfaceFormat::DebugContext);
  setFormat(format);

  // Auto update opengl drawing
  QTimer* timer = new QTimer( this );
  connect(timer, &QTimer::timeout, this, &Viewer::updateScene);
  timer->start(5);

  // Create Scene
  _scene = std::make_shared<TerrainScene>();
  //_scene = std::make_shared<PatchTessTestScene>();
//  _scene = std::make_shared<TessTerrainScene>();
//  _scene = std::make_shared<TessTestScene>();
//  _scene = std::make_shared<TestInstanciationScene>();

  // If a dock is related to the current scene, create it
  QDockWidget *dock = _scene->getDock();
  if (dock) {
    _mainWindow->addDockWidget(Qt::RightDockWidgetArea, dock);
  }

  if(MainWindow * mw = dynamic_cast<MainWindow *>(_mainWindow))
    _scene->connectToMainWindow(mw);
}

Viewer::~Viewer(){
  // Clean GL Objects from scene
  makeCurrent();
  _scene->clean();
  doneCurrent();
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

  // Start timer
  _time.start();
  _previousTime = _time.elapsed();

  // Initialize scene
  _scene->setFuncs(_funcs);
  _scene->initialize();
}

void Viewer::paintGL(){
  // Render Scene
  _scene->render();

  _frameNumber++;
}

void Viewer::resizeGL(int width, int height){
  // Resize Scene
  _scene->resize(width, height);
}

void Viewer::updateScene() {
  // Update time (in ms)
  float time = _time.elapsed();
  _dt = time - _previousTime;
  _previousTime = time;
  updateFPSCount();

  // Update Scene
  _scene->update(_dt);

  update();
}

void Viewer::mousePressEvent(QMouseEvent *e){
  setFocus();
  _scene->mousePressEvent(e);
}

void Viewer::mouseReleaseEvent(QMouseEvent *e){
  _scene->mouseReleaseEvent(e);
}

void Viewer::mouseMoveEvent(QMouseEvent *e){
  _scene->mouseMoveEvent(e);
}

void Viewer::wheelEvent(QWheelEvent *e){
  _scene->wheelEvent(e);
}

void Viewer::keyPressEvent(QKeyEvent *e) {
  _scene->keyPressEvent(e);
  switch (e->key())
  {
    case Qt::Key_Escape:
      QCoreApplication::instance()->quit();
      break;
    default:
      QOpenGLWidget::keyPressEvent(e);
  }
}

void Viewer::keyReleaseEvent(QKeyEvent *e) {
  _scene->keyReleaseEvent(e);
  QOpenGLWidget::keyReleaseEvent(e);
}

void Viewer::focusOutEvent(QFocusEvent * event) {
  _scene->focusOutEvent(event);
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
