#ifndef DIRECTIONAL_LIGHT_H
#define DIRECTIONAL_LIGHT_H

#include "Eigen/Eigen"

class DirectionalLight {

public:

  DirectionalLight();
  DirectionalLight(Eigen::Vector3f position, Eigen::Vector3f direction, Eigen::Vector3f color, float intensity);
  virtual ~DirectionalLight();

  // getters
  Eigen::Vector3f getPosition();
  Eigen::Vector3f getDirection();
  Eigen::Vector3f getColor();
  float getIntensity();

  // setters
  void setPosition(Eigen::Vector3f position);
  void setDirection(Eigen::Vector3f direction);
  void setColor(Eigen::Vector3f color);
  void setIntensity(float intensity);

private:
    Eigen::Vector3f _position = Eigen::Vector3f(1.f, 1.f, 1.f);
    Eigen::Vector3f _direction = Eigen::Vector3f(1.f, 1.f, 1.f);
    Eigen::Vector3f _color = Eigen::Vector3f(1.f, 1.f, 1.f);
    float _intensity = 1.f;
};

#endif
