#ifndef TERRAIN_TINTIN_SLIDER_H
#define TERRAIN_TINTIN_SLIDER_H

#include <iostream>

#include <QFrame>
#include <QLabel>
#include <QDoubleSpinBox>
#include <QSlider>
#include <QHBoxLayout>

class VariableOption : public QWidget {
  Q_OBJECT
 public:
 VariableOption()
   : _layout(nullptr), _label(nullptr), _spinBox(nullptr), _slider(nullptr) {}
  
 VariableOption(std::string label, float value, float min, float max, float step)
   : _min(min), _max(max), _step(step) {
    _layout = new QHBoxLayout;
    _layout->setContentsMargins(0,0,0,0);
    _label = new QLabel(label.c_str(), this);
    _spinBox = new QDoubleSpinBox(this);
    _spinBox->setRange(min, max);
    _spinBox->setSingleStep(step);
    _spinBox->setValue(value);
    //    _spinBox->setDecimals(1);
    _slider = new QSlider(this);
    _slider->setOrientation(Qt::Horizontal);
    _slider->setSingleStep(1);
    _slider->setMinimum(0);
    _slider->setMaximum((int)((max-min)/step));
    _slider->setValue((int)((value-min)/step));

    _layout->addWidget(_label);
    _layout->addWidget(_spinBox);
    _layout->addWidget(_slider);

    this->setLayout(_layout);

    QObject::connect(_slider, SIGNAL(valueChanged(int)),
		     this, SLOT(sliderValueChanged(int)));
    QObject::connect(_spinBox, SIGNAL(valueChanged(double)),
		     this, SLOT(spinBoxValueChanged(double)));
  }

  double value() {
    return _spinBox->value();
  }
  
  private slots :
    void sliderValueChanged(int val) {
      _spinBox->blockSignals(true);
      _spinBox->setValue(_min+_step*val);
      _spinBox->blockSignals(false);
      emit valueChanged(_min+_step*val);
    }
   
    void spinBoxValueChanged(double val) {
      _slider->blockSignals(true);
      _slider->setValue((val-_min)/_step);
      _slider->blockSignals(false);
      emit valueChanged(val);
    }
   
 signals :
    void valueChanged(double);
  
 
 private :
    QLayout * _layout;
    QLabel * _label;
    QDoubleSpinBox * _spinBox;
    QSlider * _slider;

    float _min, _max, _step;
};

#endif //TERRAIN_TINTIN_SLIDER_H
