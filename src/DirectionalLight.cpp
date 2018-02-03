#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(Eigen::Vector3f position, Eigen::Vector3f direction, Eigen::Vector3f color, float intensity)
:_position(position), _direction(direction), _color(color), _intensity(intensity)
 {}

DirectionalLight::~DirectionalLight()
{}

Eigen::Vector3f DirectionalLight::getPosition() {
  return _position;
}

Eigen::Vector3f DirectionalLight::getDirection() {
  return _direction;
}

Eigen::Vector3f DirectionalLight::getColor() {
  return _color;
}

float DirectionalLight::getIntensity() {
  return _intensity;
}

void DirectionalLight::setPosition(Eigen::Vector3f position) {
  _position = position;
}

void DirectionalLight::setDirection(Eigen::Vector3f direction) {
  _direction = direction;
}

void DirectionalLight::setColor(Eigen::Vector3f color) {
  _color = color;
}

void DirectionalLight::setIntensity(float intensity) {
  _intensity = intensity;
}
