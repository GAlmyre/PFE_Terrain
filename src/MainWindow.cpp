// MainWindow.cpp

#include <QtWidgets>

#include "MainWindow.hpp"
#include "ui_MainWindow.h"
#include <iostream>

MainWindow::MainWindow(QWidget *parent) :
  QMainWindow(parent),
  _ui(new Ui::MainWindow)
{
  _ui->setupUi(this);

  _viewer = new Viewer(this);

  createActions();
  createMenu();
  setWindowTitle(tr("TerrainTintin"));

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

  _openAction = new QAction(tr("&Open"), this);
  _openAction->setStatusTip(tr("Open a file from your computer"));
  connect(_openAction, SIGNAL(triggered()), this, SLOT(open()));

  _exitAction = new QAction(tr("&Exit"), this);
  _exitAction->setStatusTip(tr("Exit the program"));
  connect(_exitAction, SIGNAL(triggered()), this, SLOT(exit()));
}

void MainWindow::createMenu() {

  _fileMenu = menuBar()->addMenu(tr("&File"));
  _fileMenu->addAction(_openAction);
  _fileMenu->addSeparator();
  _fileMenu->addAction(_exitAction);
}

// Handles the opening of a 3D file
void MainWindow::open() {

  QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"),"../data", tr("3D files (*.obj *pgm3d)"), Q_NULLPTR, QFileDialog::Options(QFileDialog::DontUseNativeDialog));

  QFileInfo fileInfo(fileName);
  QString fileExtension = fileInfo.suffix();

  std::cout << "opened file :" << fileName.toStdString() << '\n';
}

void MainWindow::exit() {

  this->close();
}
