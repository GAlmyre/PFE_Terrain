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
  QMenu* _viewMenu;
  QMenu* _optionMenu;
  QAction* _loadHeightmapAction;
  QAction* _loadTextureAction;
  QAction* _exitAction;
  QAction* _toggleFogAction;

  // mutually exclusive
  QActionGroup* _tessellationMethodsGroup;
  QAction* _autoTessellationAction;
  QAction* _customTessellationAction;

  void createMenu();
  void createActions();

private slots:
  void exit();
  void loadHeightMap();
  void loadTexture();
  void toggleFog();
  void tessellationMethod();

 signals:
  void loadedHeightMap(const QImage&);
  void loadedTexture(const QImage&);
};

#endif // MAINWINDOW_HPP
