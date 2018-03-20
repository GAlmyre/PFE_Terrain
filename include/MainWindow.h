// MainWindow.hpp

#ifndef MAINWINDOW_HPP
#define MAINWINDOW_HPP

#include <QMainWindow>
#include <QWidget>
#include <QSlider>

class QActionGroup;
class Viewer;

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

  void loadHeightMap(const QString &filename);

protected:
  void keyPressEvent(QKeyEvent *e);

private:
  Ui::MainWindow *_ui;
  Viewer* _viewer;
  QMenu* _fileMenu;
  QMenu *_toolsMenu;
  QAction* _loadHeightmapAction;
  QAction* _loadTextureAction;
  QAction* _exitAction;

  void createMenu();
  void createActions();

public:
  QAction *_toggleNormalsAction;
  QAction *_toggleGradAction;
  QAction *_toggleBenchAction;

private slots:
  void loadHeightMap();
  void loadTexture();

 signals:
  void loadedHeightMap(const QImage&);
  void loadedHeightMap(const QString&);
  void loadedTexture(const QImage&);
  void toggledNormals(bool);
  void toggledGrab(bool);
  void toggledBench(bool);
};

#endif // MAINWINDOW_HPP
