#include <QtTest/QtTest>
#include "FreeFlyCamera.h"

class TestFreeFlyCamera: public QObject
{
    Q_OBJECT
private:
  FreeFlyCamera _camera;

public:
  TestFreeFlyCamera() {
    _camera = FreeFlyCamera(Eigen::Vector3f(1.0,1.0,1.0), Eigen::Vector3f(1.0,0.0,0.0), 800, 600);
  }

private slots:
  void position(){
    QVERIFY(_camera.position() == Eigen::Vector3f(1.0,1.0,1.0));
  }
  void snapped(){
    _camera.snapToGround(true);
    QVERIFY(_camera.snappedToGround() == true);
    _camera.snapToGround(false);
    QVERIFY(_camera.snappedToGround() == false);
  }

};

QTEST_MAIN(TestFreeFlyCamera)
#include "FreeFlyCameraTest.moc"
