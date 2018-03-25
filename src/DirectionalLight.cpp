#include "DirectionalLight.h"

DirectionalLight::DirectionalLight() {}

DirectionalLight::DirectionalLight(Eigen::Vector3f observerPosition, float distanceToObserver, float azimuth, float altitude, Eigen::Vector3f color)
  :_observerPosition(observerPosition), _distToObserver(distanceToObserver), _azimuth(azimuth), _altitude(altitude), _color(color)
{
  computeDirection();
}

DirectionalLight::~DirectionalLight()
{}

Eigen::Vector3f DirectionalLight::getObserverPosition() {
  return _observerPosition;
}

float DirectionalLight::getDistanceToObserver() {
  return _distToObserver;
}

Eigen::Vector3f DirectionalLight::getDirection() {
  return _direction;
}

float DirectionalLight::getAzimuth() {
  return _azimuth;
}

float DirectionalLight::getAltitude() {
  return _altitude;
}

Eigen::Vector3f DirectionalLight::getColor() {
  return _color;
}


void DirectionalLight::setObserverPosition(Eigen::Vector3f observerPosition) {
  _observerPosition = observerPosition;
}

void DirectionalLight::setDistanceToObserver(float dist) {
  _distToObserver = dist;
}

void DirectionalLight::setAzimuth(float azimuth) {
  _azimuth = azimuth;
  computeDirection();
}

void DirectionalLight::setAltitude(float altitude) {
  _altitude = altitude;
  computeDirection();
}

void DirectionalLight::setColor(Eigen::Vector3f color) {
  _color = color;
}

void DirectionalLight::computeDirection() {
  float x = _distToObserver * cos(_altitude) * cos(_azimuth);
  float y = _distToObserver * sin(_altitude);
  float z = _distToObserver * cos(_altitude) * sin(_azimuth);
  _direction = Eigen::Vector3f(x, y, z);
  _direction.normalize();
}
