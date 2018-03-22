#include "BenchmarkWindow.h"

#include "MainWindow.h"

#include <iostream>
#include <QtWidgets>

BenchmarkWindow::BenchmarkWindow(QWidget *parent, MainWindow *mainWindow)
        : QFrame(parent, Qt::WindowStaysOnTopHint | Qt::Window | Qt::CustomizeWindowHint | Qt::WindowTitleHint),
          _mainWindow(mainWindow), _currentData(0, 0, 0, 0)
{
  setWindowTitle("Benchmarking");
  setVisible(false);

  _primGenLabel = new QLabel("0", this);
  _gpuTimeLabel = new QLabel("0", this);
  _cpuTimeLabel = new QLabel("0", this);
  _lodTimeLabel = new QLabel("0", this);

  auto *hLayout = new QVBoxLayout(this);

  auto *formLayout = new QFormLayout;
  formLayout->addRow("Triangles", _primGenLabel);
  formLayout->addRow("GPU Time", _gpuTimeLabel);
  formLayout->addRow("CPU Time", _cpuTimeLabel);
  formLayout->addRow("LOD Time", _lodTimeLabel);

  auto *vLayout = new QHBoxLayout;
  auto *buttonStop = new QPushButton("Close", this);
  auto *buttonSave = new QPushButton("Record", this);
  auto *savedLabel = new QLabel("Not Recording", this);

  buttonSave->setCheckable(true);
  vLayout->addWidget(buttonStop);
  vLayout->addWidget(buttonSave);

  hLayout->addLayout(formLayout);
  hLayout->addLayout(vLayout);
  hLayout->addWidget(savedLabel);

  connect(buttonStop, &QAbstractButton::clicked, [this] {
    setVisible(false);
    _benchmark.clear();
    _mainWindow->_toggleBenchAction->setChecked(false);
  });

  connect(buttonSave, &QAbstractButton::toggled, [=] (bool checked) {
    if (checked) {
      savedLabel->setText("Recording");
      buttonSave->setText("Stop");
      _benchmark.clear();
    } else {
      QString fileName = writeBenchmarkData();
      savedLabel->setText("Not Recording");
      buttonSave->setText("Record");
      _benchmark.clear();
    }
  });

  connect(_mainWindow, &MainWindow::toggledBench, [this](bool checked) {
    setVisible(checked);
    _benchmark.clear();
  });

  _saveDialog = new QFileDialog(this);
  _saveDialog->setFileMode(QFileDialog::AnyFile);
  _saveDialog->setNameFilter(tr("Text Files (*.txt)"));
  _saveDialog->setAcceptMode(QFileDialog::AcceptSave);
  _saveDialog->setOption(QFileDialog::DontUseNativeDialog);
  _saveDialog->setViewMode(QFileDialog::List);
  _saveDialog->setDirectory("../data/bench/");
}

QString BenchmarkWindow::writeBenchmarkData() {
  QString fileName = QFileDialog::getSaveFileName(this, tr("Save Benchmark File"),
                                                  "../data/bench/",
                                                  tr("Text files (*.txt)"), nullptr, QFileDialog::DontUseNativeDialog);
  if (!fileName.isNull()) {
    QFile file(fileName);
    if (file.open(QIODevice::WriteOnly)) {
      QTextStream out(&file);
      out.setRealNumberNotation(QTextStream::FixedNotation);
      out.setRealNumberPrecision(6);

      for (auto &stat : _benchmark) {
        out << stat.primitiveGenerated << ", " << stat.gpuTime << ", " << stat.cpuTime << ", " << stat.lodTime << "\n";
      }

      file.close();
    }
  }

  return fileName;
}

void BenchmarkWindow::addNewData(const Data &data) {
  if (isVisible()) {
    _benchmark.push_back(data);

    const float sustain = 0.1;
    const float b = 1.f - sustain;

    _currentData.primitiveGenerated = data.primitiveGenerated;
    _currentData.cpuTime = data.cpuTime * sustain + _currentData.cpuTime * b;
    _currentData.gpuTime = data.gpuTime * sustain + _currentData.gpuTime * b;
    _currentData.lodTime = data.lodTime * sustain + _currentData.lodTime * b;

    _primGenLabel->setText(QString::number(_currentData.primitiveGenerated));
    _gpuTimeLabel->setText(QString::number(_currentData.gpuTime, 'f', 3) + " ms");
    _cpuTimeLabel->setText(QString::number(_currentData.cpuTime, 'f', 3) + " ms");
    _lodTimeLabel->setText(QString::number(_currentData.lodTime, 'f', 3) + " ms");
  }
}
