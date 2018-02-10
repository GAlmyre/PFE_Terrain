#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "Eigen/Eigen"

class DirectionalLight {

 public:

  DirectionalLight();
  DirectionalLight(Eigen::Vector3f observerPosition, float distanceToObserver, float azimuth, float altitude, Eigen::Vector3f color);
  virtual ~DirectionalLight();

  // getters
  Eigen::Vector3f getObserverPosition();
  float getDistanceToObserver();
  Eigen::Vector3f getDirection();
  float getAzimuth();
  float getAltitude();
  Eigen::Vector3f getColor();

  // setters
  void setObserverPosition(Eigen::Vector3f position);
  void setDistanceToObserver(float dist);
  void setDirection(Eigen::Vector3f direction);
  void setAzimuth(float azimuth);
  void setAltitude(float altitude);
  void setColor(Eigen::Vector3f color);

 private:

  void computeDirection();
  
  Eigen::Vector3f _observerPosition = Eigen::Vector3f(0.f, 0.f, 0.f);
  float _distToObserver = 10.f;
    
  bool _updateDirection = false;
  Eigen::Vector3f _direction = Eigen::Vector3f(1.f, 0.f, 0.f);

  float _azimuth = 0.f;
  float _altitude = 0.f;
    
    Eigen::Vector3f _color = Eigen::Vector3f(1.f, 1.f, 1.f);
};

#endif
