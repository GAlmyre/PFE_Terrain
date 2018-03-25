#include <QtTest/QtTest>
#include "DirectionalLight.h"
#include "iostream"

class TestDirectionalLight: public QObject
{
    Q_OBJECT
private:
  DirectionalLight _light;

public:
  TestDirectionalLight() {
    _light = DirectionalLight(Eigen::Vector3f(1.0,1.0,1.0), 100.0, 45.0, 10.0, Eigen::Vector3f(1.0,1.0,1.0));
  }

private slots:
  void distanceToObserver(){
    QVERIFY(_light.getDistanceToObserver() == 100.0);
  }
  void observerPosition(){
    QVERIFY(_light.getObserverPosition() == Eigen::Vector3f(1.0,1.0,1.0));
  }
  void azimuth(){
    QVERIFY(_light.getAzimuth() == 45.0);
  }
  void altitude(){
    QVERIFY(_light.getAltitude() == 10.0);
  }
  void color(){
    QVERIFY(_light.getColor() == Eigen::Vector3f(1.0,1.0,1.0));
  }
  void computeDirection(){
    Eigen::Vector3f result = Eigen::Vector3f(98.4714, 17.3648, 1.3578);
    QVERIFY(_light.getDirection().x() - result.x() <= 0.001);
    QVERIFY(_light.getDirection().y() - result.y() <= 0.001);
    QVERIFY(_light.getDirection().z() - result.z() <= 0.001);
  }
  void setDistanceToObserver(){
    _light.setDistanceToObserver(10.0);
    QVERIFY(_light.getDistanceToObserver() == 10.0);
  }
  void setObserverPosition(){
    _light.setObserverPosition(Eigen::Vector3f(1.0,2.0,3.0));
    QVERIFY(_light.getObserverPosition() == Eigen::Vector3f(1.0,2.0,3.0));
  }
  void setAzimuth(){
    _light.setAzimuth(1.0);
    QVERIFY(_light.getAzimuth() == 1.0);
  }
  void setAltitude(){
    _light.setAltitude(42.0);
    QVERIFY(_light.getAltitude() == 42.0);
  }
  void setColor(){
    _light.setColor(Eigen::Vector3f(3.0,2.0,1.0));
    QVERIFY(_light.getColor() == Eigen::Vector3f(3.0,2.0,1.0));
  }

};

QTEST_MAIN(TestDirectionalLight)
#include "DirectionalLightTest.moc"
