/*
 Copyright 2015 Simon Boyé.

 This Source Code Form is subject to the terms of the Mozilla Public
 License, v. 2.0. If a copy of the MPL was not distributed with this
 file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#include "Trackball.h"

Trackball::Trackball() :
        _scnCenter(Eigen::Vector3f::Zero()),
        _scnDistance(3.),
        _scnRadius(1.),
        _scnOrientation(1., 0., 0., 0.),
        _state(Idle) {
}


const Eigen::Vector3f& Trackball::sceneCenter() const {
  return _scnCenter;
}

float Trackball::sceneDistance() const {
  return _scnDistance;
}

float Trackball::sceneRadius() const {
  return _scnRadius;
}


const Eigen::Quaternionf& Trackball::sceneOrientation() const {
  return _scnOrientation;
}

Eigen::Matrix4f Trackball::computeViewMatrix() const {
  Eigen::Vector3f scnCamera = _scnCenter;
  scnCamera += _scnOrientation * Eigen::Vector3f::UnitZ() * _scnDistance;

  return Eigen::Affine3f(
          _scnOrientation.inverse() * Eigen::Translation3f(-scnCamera)).matrix();
}

void Trackball::setSceneCenter(const Eigen::Vector3f& scnCenter) {
  _scnCenter = scnCenter;
}

void Trackball::setSceneDistance(float scnDistance) {
  _scnDistance = scnDistance;
}

void Trackball::setSceneRadius(float scnRadius) {
  _scnRadius = scnRadius;
}

void Trackball::setSceneOrientation(const Eigen::Quaternionf& scnOrientation) {
  _scnOrientation = scnOrientation;
}

bool Trackball::isIdle() const {
  return _state == Idle;
}

void Trackball::rotate(const Eigen::Quaternionf& rot) {
  _scnOrientation *= rot;
}

bool Trackball::isRotating() const {
  return _state == Rotating;
}

void Trackball::startRotation(const Eigen::Vector2f& scrPos) {
  assert(_state == Idle);
  _state = Rotating;
  _scrMouseInit = scrPos;
  _scnOrientInit = _scnOrientation;
}

void Trackball::dragRotate(const Eigen::Vector2f& scrPos) {
  assert(_state == Rotating);
  _scnOrientation = computeRotation(scrPos);
}

void Trackball::cancelRotation() {
  assert(_state == Rotating);
  _state = Idle;
  _scnOrientation = _scnOrientInit;
}

void Trackball::endRotation() {
  assert(_state == Rotating);
  _state = Idle;
}

bool Trackball::isTranslating() const {
  return _state == Translating;
}

void Trackball::startTranslation(const Eigen::Vector2f& scrPos) {
  assert(_state == Idle);
  _state = Translating;
  _scrMouseInit = scrPos;
  _scnCenterInit = _scnCenter;
}

void Trackball::dragTranslate(const Eigen::Vector2f& scrPos) {
  assert(_state == Translating);
//  _scnCenter = computeTranslation(scrPos);
}

void Trackball::cancelTranslation() {
  assert(_state == Translating);
  _state = Idle;
  _scnCenter = _scnCenterInit;
}

void Trackball::endTranslation() {
  assert(_state == Translating);
  _state = Idle;
}

void Trackball::zoom(float factor) {
  _scnDistance /= factor;
}

void Trackball::grow(float factor) {
  _scnRadius *= factor;
}

void Trackball::dollyZoom(float factor) {
  _scnDistance /= factor;
}

Eigen::Quaternionf Trackball::computeRotation(const Eigen::Vector2f& scrPos) const {
  Eigen::Vector2f v = (_scrMouseInit - scrPos) * m_sensitivity;
  Eigen::Vector3f x = Eigen::Vector3f::UnitX();
  Eigen::Vector3f y = Eigen::Vector3f::UnitY();
  return _scnOrientInit * Eigen::Quaternionf(Eigen::AngleAxisf(v.x(), y))
         * Eigen::Quaternionf(Eigen::AngleAxisf(v.y(), x));
}

void Trackball::update(float dt) {
  m_viewMatrix = computeViewMatrix();
}

Trackball::State Trackball::state() const {
  return _state;
}

void Trackball::screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const {

}

void Trackball::processKeyRelease(Key key) {

}

void Trackball::processKeyPress(Key key) {

}

void Trackball::processMousePress(int mouseX, int mouseY) {

}

void Trackball::processMouseRelease() {

}

void Trackball::processMouseMove(int mouseX, int mouseY) {

}

void Trackball::processMouseScroll(float yoffset) {

}

void Trackball::stopMovement() {
  _state = Idle;
}
