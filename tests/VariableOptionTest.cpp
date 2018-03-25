#include <QtTest/QtTest>
#include "VariableOption.h"

class TestVariableOption: public QObject
{
    Q_OBJECT
private slots:
  void value(){
    VariableOption vo("label", 50.2, 1, 78, 0.01);
    QVERIFY(vo.value() == 50.2);
  }
};

QTEST_MAIN(TestVariableOption)
#include "VariableOptionTest.moc"
