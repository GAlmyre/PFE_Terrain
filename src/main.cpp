// main.cpp
#include <QApplication>
#include <QDesktopWidget>

#include "MainWindow.h"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>

// #define DEBUG

using namespace std;

int main(int argc, char *argv[]) {
  
  QApplication app(argc, argv);
  MainWindow window;

  unsigned int height = 900;
  float ratio = 16./9;
  window.resize(ratio*height,height);
  window.show();
  
  return app.exec();
}
