#ifndef TERRAIN_TINTIN_COMBOBOXOPTION_H
#define TERRAIN_TINTIN_COMBOBOXOPTION_H

#include <QFrame>
#include <QLabel>
#include <QComboBox>
#include <QHBoxLayout>

class ComboBoxOption : public QWidget {
  Q_OBJECT
 public:
 ComboBoxOption()
   : _layout(nullptr), _label(nullptr), _comboBox(nullptr) {}
  
 ComboBoxOption(std::string label) {
    _layout = new QHBoxLayout;
    _layout->setContentsMargins(0,0,0,0);
    _label = new QLabel(label.c_str(), this);

    _comboBox = new QComboBox(this);
    _comboBox->setEditable(false);
    
    _layout->addWidget(_label);
    _layout->addWidget(_comboBox);
    
    this->setLayout(_layout);

    QObject::connect(_comboBox, SIGNAL(activated(int)),
		     this, SLOT(comboBoxActivated(int)));
  }

 void addItem(std::string name, int data){
   _comboBox->addItem(name.c_str(), data); 
 }

 void setCurrentIndex(int ind){
   _comboBox->setCurrentIndex(ind);
 }
 
  private slots :
    void comboBoxActivated(int val) {emit activated(_comboBox->itemData(val).toInt());}
   
 signals :
    void activated(int);
 
 private :
    QLayout * _layout;
    QLabel * _label;
    QComboBox * _comboBox;
};

#endif //TERRAIN_TINTIN_COMBOBOXOPTION_H
