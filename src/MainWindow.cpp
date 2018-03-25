// MainWindow.cpp

#include <MainWindow.h>
#include "ui_MainWindow.h"

#include <Viewer.h>
#include <iostream>
#include <sstream>
#include <Eigen/Dense>

#include "CImg/CImg.h"

using cimg_library::CImg;
using cimg_library::CImgIOException;
using cimg_library::CImgList;

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  _ui(new Ui::MainWindow)
{
  _ui->setupUi(this);

  _viewer = new Viewer(this);

  statusBar()->showMessage("Hello !");
  createActions();
  createMenu();
  setWindowTitle(tr("TerrainTintin"));

  connect(_viewer, &Viewer::fpsChanged, [&](float fps) {
    statusBar()->showMessage(QString::number(std::floor(fps)) + " fps"); });

  this->setCentralWidget(_viewer);
}

MainWindow::~MainWindow()
{
  delete _ui;
}

void MainWindow::keyPressEvent(QKeyEvent *e){

}

void MainWindow::createActions() {
  _loadHeightmapAction = new QAction(tr("&Load heightmap"), this);
  _loadHeightmapAction->setStatusTip(tr("Load the selected heightmap"));
  connect(_loadHeightmapAction, SIGNAL(triggered()), this, SLOT(loadHeightMap()));

  _loadTextureAction = new QAction(tr("&Load texture"), this);
  _loadTextureAction->setStatusTip(tr("Load the selected texture"));
  connect(_loadTextureAction, SIGNAL(triggered()), this, SLOT(loadTexture()));

  _exitAction = new QAction(tr("&Exit"), this);
  _exitAction->setStatusTip(tr("Exit the program"));
  connect(_exitAction, SIGNAL(triggered()), QCoreApplication::instance(), SLOT(quit()));

  _toggleBenchAction = new QAction(tr("&Benchmark"), this);
  _toggleBenchAction->setStatusTip(tr("Enable/disable Benchmarking"));
  _toggleBenchAction->setCheckable(true);
  connect(_toggleBenchAction, &QAction::toggled, [this] (bool checked) {
    emit toggledBench(checked);
  });

  _toggleNormalsAction = new QAction(tr("&Show Normals"), this);
  _toggleNormalsAction->setStatusTip(tr("Show normals for debug purpose"));
  _toggleNormalsAction->setCheckable(true);
  connect(_toggleNormalsAction, &QAction::toggled, [this] (bool checked) {
    emit toggledNormals(checked);
  });

  _toggleSnapAction = new QAction(tr("&Snap camera to ground"), this);
  _toggleSnapAction->setStatusTip(tr("Make the camera flying on top of the ground"));
  _toggleSnapAction->setCheckable(true);
  connect(_toggleSnapAction, &QAction::toggled, [this] (bool checked) {
    emit toggledSnap(checked);
  });
}

void MainWindow::createMenu() {
  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_loadHeightmapAction);
  _fileMenu->addAction(_loadTextureAction);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_exitAction);

  _toolsMenu = menuBar()->addMenu(tr("&Tools"));
  _toolsMenu->addAction(_toggleBenchAction);
  _toolsMenu->addAction(_toggleNormalsAction);
  _toolsMenu->addAction(_toggleSnapAction);
}

// Handles the opening of a heightmap
void MainWindow::loadHeightMap() {

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../data", tr("HeightMap (*.png *.bmp *.pgm *.tga *.ppm)"), Q_NULLPTR, QFileDialog::Options(QFileDialog::DontUseNativeDialog));
  if (!fileName.isNull()) loadHeightMap(fileName);
}

void MainWindow::loadHeightMap(const QString &filename) {
  QFileInfo fileInfo(filename);
  QString fileExtension = fileInfo.suffix();

  emit loadedHeightMap(filename);
}


// Handles the opening of a texture
void MainWindow::loadTexture() {

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../data", tr("Texture (*.png *.bmp *.pgm *.tga *.ppm)"), Q_NULLPTR, QFileDialog::Options(QFileDialog::DontUseNativeDialog));
  if (fileName.isNull()) return;

  QFileInfo fileInfo(fileName);
  QString fileExtension = fileInfo.suffix();

  QImage texture;
  if (!texture.load(fileName)) {
    QMessageBox::about(this, "Error while loading texture", "Unable to load the image file located at " + fileName);
    return;
  }
  std::cout << "opened texture file :" << fileName.toStdString() << '\n';

  emit loadedTexture(texture);
}
