// MainWindow.cpp

#include <MainWindow.h>
#include "ui_MainWindow.h"

#include <Viewer.h>
#include <iostream>

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

  _viewer->eventFromParent(e);
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
  connect(_exitAction, SIGNAL(triggered()), this, SLOT(exit()));

  _toggleFogAction = new QAction(tr("&Toggle Fog"), this);
  _toggleFogAction->setStatusTip(tr("Enable/disable the distance fog"));
  _toggleFogAction->setCheckable(true);
  connect(_toggleFogAction, SIGNAL(toggled(bool)), this, SLOT(toggleFog()));

  // creates mutually exclusive toggled Options to choose the tessellation method
  _autoTessellationAction = new QAction(tr("&Automatic Tessellation"), this);
  _autoTessellationAction->setStatusTip(tr("Use openGL's automatic tessellation"));
  _autoTessellationAction->setCheckable(true);
  connect(_autoTessellationAction, SIGNAL(toggled(bool)), this, SLOT(tessellationMethod()));

  _customTessellationAction = new QAction(tr("&Custom Tessellation"), this);
  _customTessellationAction->setStatusTip(tr("Use custom tessellation"));
  _customTessellationAction->setCheckable(true);
  connect(_customTessellationAction, SIGNAL(toggled(bool)), this, SLOT(tessellationMethod()));

  _tessellationMethodsGroup = new QActionGroup(this);
  _tessellationMethodsGroup->addAction(_autoTessellationAction);
  _tessellationMethodsGroup->addAction(_customTessellationAction);
  _autoTessellationAction->setChecked(true);

}

void MainWindow::createMenu() {

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_loadHeightmapAction);
  _fileMenu->addAction(_loadTextureAction);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_exitAction);

  _viewMenu = menuBar()->addMenu(tr("&View"));
  _viewMenu->addAction(_toggleFogAction);

  _optionMenu = menuBar()->addMenu(tr("&Options"));
  _optionMenu->addAction(_autoTessellationAction);
  _optionMenu->addAction(_customTessellationAction);
}

// Handles the opening of a heightmap
void MainWindow::loadHeightMap() {

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../data", tr("HeightMap (*.png *.bmp *.pgm *.tga *.ppm)"), Q_NULLPTR, QFileDialog::Options(QFileDialog::DontUseNativeDialog));

  QFileInfo fileInfo(fileName);
  QString fileExtension = fileInfo.suffix();

  std::cout << "opened heightmap file :" << fileName.toStdString() << '\n';
}

// Handles the opening of a texture
void MainWindow::loadTexture() {

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../data", tr("Texture (*.png *.bmp *.pgm *.tga *.ppm)"), Q_NULLPTR, QFileDialog::Options(QFileDialog::DontUseNativeDialog));

  QFileInfo fileInfo(fileName);
  QString fileExtension = fileInfo.suffix();

  std::cout << "opened texture file :" << fileName.toStdString() << '\n';
}

void MainWindow::toggleFog() {

  std::cout << "Distance fog : " << _toggleFogAction->isChecked() << '\n';
}

void MainWindow::tessellationMethod(){

  std::cout << "Auto tessellation : " << _autoTessellationAction->isChecked() << '\n';
  std::cout << "Custom tessellation : " << _customTessellationAction->isChecked() << '\n';
}

// Closes the main window
void MainWindow::exit() {
  this->close();
}
