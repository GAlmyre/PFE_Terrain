// main.cpp
#include <QApplication>
#include <QDesktopWidget>

#include "MainWindow.hpp"

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <string.h>

// #define DEBUG

using namespace std;

int main(int argc, char *argv[]) {

  QApplication app(argc, argv);
  MainWindow window;

  window.resize(1300,1300);
  window.show();

  return app.exec();
}
