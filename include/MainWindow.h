// MainWindow.hpp

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QSlider>

#include "Viewer.hpp"

namespace Ui {
  class MainWindow;
}

// The main window for the application, it holds the menu as well as the viewer that will render the models.
class MainWindow : public QMainWindow
{
  Q_OBJECT

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

protected:
  void keyPressEvent(QKeyEvent *e);

private:
  Ui::MainWindow *_ui;
  Viewer* _viewer;
  QMenu* _fileMenu;
  QAction* _loadHeightmapAction;
  QAction* _loadTextureAction;
  QAction* _exitAction;

  void createMenu();
  void createActions();

private slots:
  void exit();
  void loadHeightMap();
  void loadTexture();
};

#endif // MAINWINDOW_HPP
