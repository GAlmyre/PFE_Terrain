#ifndef TERRAINTINTIN_BENCHMARKWINDOW_H
#define TERRAINTINTIN_BENCHMARKWINDOW_H

#include <QFrame>

class QLabel;
class MainWindow;
class QFileDialog;

class BenchmarkWindow : public QFrame {
  Q_OBJECT
public:
  struct Data {
    Data(unsigned int primitiveGenerated, double gpuTime, double cpuTime, double lodTime)
            : primitiveGenerated(primitiveGenerated), gpuTime(gpuTime), cpuTime(cpuTime), lodTime(lodTime) {}
    unsigned int primitiveGenerated;
    double gpuTime;
    double cpuTime;
    double lodTime;
  };
public:
  explicit BenchmarkWindow(QWidget *parent, MainWindow *mainWindow);

  void addNewData(const Data &data);

private:
  QString writeBenchmarkData();

private:
  MainWindow *_mainWindow;

  QLabel *_primGenLabel = nullptr;
  QLabel *_gpuTimeLabel = nullptr;
  QLabel *_cpuTimeLabel = nullptr;
  QLabel *_lodTimeLabel = nullptr;

  std::vector<Data> _benchmark;
  Data _currentData;

signals:
    void newData(const Data &);
};


#endif //TERRAINTINTIN_BENCHMARKWINDOW_H
