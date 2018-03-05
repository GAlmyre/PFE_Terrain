#include <FreeFlyCamera.h>

#include <iostream>
#include <algorithm>

#include <utils.h>

using namespace Eigen;

FreeFlyCamera::FreeFlyCamera()
  : m_position(Vector3f::Zero()), m_direction(Vector3f::UnitZ()), m_yaw(0), m_pitch(0), m_width(800), m_height(600)
  , m_fovy(M_PI / 3.f), m_near(0.1), m_far(50000), m_mouseOffset(Vector2f(0.f, 0.f)), m_rotating(false)
{
  m_viewMatrix.setIdentity();
  setDirection(m_direction);

  updateProjectionMatrix();
  initOffsetBuffer();
  resetKeyStates();
}

FreeFlyCamera::FreeFlyCamera(const Eigen::Vector3f &position, const Eigen::Vector3f &direction, int width, int height)
{
  m_viewMatrix.setIdentity();

  m_mouseOffset = Vector2f(0.f, 0.f);
  
  m_position = position;
  setDirection(direction);

  m_yaw = 0;
  m_pitch = 0;

  m_width = width;
  m_height = height;

  m_fovy = M_PI / 3.f;
  m_near = 0.1;
  m_far = 50000.f;
  m_rotating = false;

  updateProjectionMatrix();
  initOffsetBuffer();
  resetKeyStates();
}

void FreeFlyCamera::setPosition(const Vector3f &position) {
  m_position = position;
}

void FreeFlyCamera::setDirection(const Vector3f &direction) {
  m_direction = -direction.normalized();
  m_pitch = asin(m_direction.y());

  if (radToDeg(m_pitch) > 89.0f)
    m_pitch = degToRad(89.0f);
  if (radToDeg(m_pitch) < -89.0f)
    m_pitch = degToRad(-89.0f);

  m_yaw = atan2(m_direction.z(), m_direction.x());
  updateViewMatrix();

}

void FreeFlyCamera::setPerspective(float fovY, float near, float far) {
  m_fovy = fovY;
  m_near = near;
  m_far = far;
  updateProjectionMatrix();
}

void FreeFlyCamera::setViewport(int width, int height) {
  m_width = width;
  m_height = height;
  updateProjectionMatrix();
}

void FreeFlyCamera::setSpeed(float speed) {
  m_speed = speed;
}

void FreeFlyCamera::updateProjectionMatrix() {
  m_ProjectionMatrix.setIdentity();
  float aspect = m_width / m_height;
  float theta = m_fovy * 0.5f;
  float range = m_far - m_near;
  float invtan = 1.f / tan(theta);

  m_ProjectionMatrix(0,0) = invtan / aspect;
  m_ProjectionMatrix(1,1) = invtan;
  m_ProjectionMatrix(2,2) = -(m_near + m_far) / range;
  m_ProjectionMatrix(3,2) = -1;
  m_ProjectionMatrix(2,3) = -2 * m_near * m_far / range;
  m_ProjectionMatrix(3,3) = 0;
}

void FreeFlyCamera::updateViewMatrix() {
  m_viewMatrix.setIdentity();

  m_direction.x() = cos(m_pitch) * cos(m_yaw);
  m_direction.y() = sin(m_pitch);
  m_direction.z() = cos(m_pitch) * sin(m_yaw);
  m_direction.normalize();

  Vector3f right = m_direction.cross(m_worldUp).normalized();
  Vector3f up = right.cross(m_direction).normalized();

  m_viewMatrix.linear().row(0) = right; // Right
  m_viewMatrix.linear().row(1) = up; // Up
  m_viewMatrix.linear().row(2) = m_direction; // Direction

  m_viewMatrix.translation() = - (m_viewMatrix.linear() * m_position);
}

const Eigen::Affine3f &FreeFlyCamera::viewMatrix() const {
  return m_viewMatrix;
}

const Eigen::Matrix4f &FreeFlyCamera::projectionMatrix() const {
  return m_ProjectionMatrix;
}

void FreeFlyCamera::screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const {
  orig = m_position;
  Vector3f localDir = Vector3f( ((2.0 * p[0] / m_width) - 1.0) * tan(m_fovy/2.0) * m_width / m_height,
                                ((2.0 * (m_height - p[1]) / m_height) - 1.0) * tan(m_fovy/2.0),
                                -1.0 );

  Vector3f right = m_direction.cross(m_worldUp).normalized();
  Vector3f up = right.cross(m_direction).normalized();

  dir = (localDir.x() * right + localDir.y() * up + localDir.z() * m_direction).normalized();
}

float FreeFlyCamera::speed() const {
  return m_speed;
}

Eigen::Vector2f FreeFlyCamera::viewport() const {
  return Eigen::Vector2f((float) m_width, (float) m_height);
}


Eigen::Vector3f FreeFlyCamera::position() {
  return m_position;
}

Eigen::Vector3f FreeFlyCamera::direction() {
  return -m_viewMatrix.linear().row(2);
}

Eigen::Vector3f FreeFlyCamera::up() {
  return m_viewMatrix.linear().row(1);
}

Eigen::Vector3f FreeFlyCamera::right() {
  return m_viewMatrix.linear().row(0);
}

void FreeFlyCamera::centerOnAABB(const AlignedBox<float, 3> &bBox, const Vector3f &dir) {
//  Vector3f front;
//  if (dir == Vector3f::Zero()) {
//    front = m_front;
//  } else {
//    //Update Yaw and pitch
//    front = dir;
//    if (dir.y() != 0) {
//      m_yaw = 90;
//      m_pitch = dir.y() * 89;
//    } else {
//      if (dir.x() == 1) {
//        m_yaw = 0;
//      } else {
//        m_yaw = 90 * dir.z() + 180 * dir.x();
//      }
//      m_pitch = 0;
//    }
//    updateCameraVectors();
//  }
//  Vector3f max = bBox.max();
//  Vector3f center = bBox.center();
//  Vector3f maxCentered = max - center;
//  float radius = maxCentered.norm();
//  float extRadius = radius;
//
//  float tanHalfFovy = m_tanHalfFovy;
//  float tanHalfFovx = tanHalfFovy * m_screenRatio;
//  float ry = extRadius / tanHalfFovy;
//  float rx = extRadius / tanHalfFovx;
//  float camRadius = std::max(ry, rx);
//
//  Vector3f camPos = -camRadius * front.normalized() + center;
//
//  m_position = camPos;
//  m_movementSpeed = camRadius / 3.f;
}

void FreeFlyCamera::update(float dt)
{
  // Update Position
  Vector3f dir = Vector3f::Zero();
  if (m_keyStates[KEY_FORWARD])  dir += direction();
  if (m_keyStates[KEY_BACKWARD]) dir -= direction();
  if (m_keyStates[KEY_RIGHT])    dir += right();
  if (m_keyStates[KEY_LEFT])     dir -= right();
  if (m_keyStates[KEY_UP])       dir += m_worldUp;
  if (m_keyStates[KEY_DOWN])     dir -= m_worldUp;

  if (dir != Vector3f::Zero())
    m_position += dir.normalized() * m_speed * dt;

  // Update Direction
  updateOffsetBuffer();
  Vector2f offset = getSmoothMouseOffset();
  if (offset != Vector2f::Zero()) {
    m_yaw   -= offset.x();
    m_pitch -= offset.y();

    if (radToDeg(m_pitch) > 89.0f)
      m_pitch = degToRad(89.0f);
    if (radToDeg(m_pitch) < -89.0f)
      m_pitch = degToRad(-89.0f);

    if (m_yaw > 2 * M_PI)
      m_yaw = m_yaw - 2 * M_PI;
    if (m_yaw < - 2 * M_PI)
      m_yaw = m_yaw + 2 * M_PI;
  }

  // Update View Matrix
  updateViewMatrix();
}

void FreeFlyCamera::processKeyPress(Key key)
{
  m_keyStates[key] = true;
}

void FreeFlyCamera::processKeyRelease(Key key)
{
  m_keyStates[key] = false;
}

void FreeFlyCamera::processMousePress(int mouseX, int mouseY)
{
  m_rotating = true;
  m_mouseLastPos.x() = mouseX;
  m_mouseLastPos.y() = -mouseY;
}

void FreeFlyCamera::processMouseRelease()
{
  m_rotating = false;
}

void FreeFlyCamera::processMouseMove(int mouseX, int mouseY)
{
  if (!m_rotating) return;

  Vector2f mousePos(mouseX, - mouseY);
  m_mouseOffset += mousePos - m_mouseLastPos;
  m_mouseLastPos = mousePos;
}

void FreeFlyCamera::processMouseScroll(float yoffset) {
//  if (m_zoom >= 1.0f && m_zoom <= 19.0f)
//    m_zoom += yoffset * std::log((21.0f - m_zoom))  * 0.001;
//  if (m_zoom <= 1.0f)
//    m_zoom = 1.0f;
//  if (m_zoom >= 19.0f)
//    m_zoom = 19.0f;
//
//  calcProjection();
}

void FreeFlyCamera::stopMovement() {
  resetKeyStates();
  m_rotating = false;
}

void FreeFlyCamera::setMouseOffsetBufferSize(size_t size) {
  m_bufferSize = size;
  initOffsetBuffer();
}

void FreeFlyCamera::initOffsetBuffer() {
  m_offsetBuffer.clear();
  m_offsetBuffer.reserve(m_bufferSize);
  for (int i = 0; i < m_bufferSize; i++) {
    m_offsetBuffer.push_back(Vector2f::Zero());
  }
  m_offsetBufferEnd = 0;
}

void FreeFlyCamera::updateOffsetBuffer()
{
  m_offsetBuffer[m_offsetBufferEnd] = m_mouseOffset * m_sensitivity;
  m_offsetBufferEnd++;
  if (m_offsetBufferEnd == m_bufferSize) m_offsetBufferEnd = 0;
  m_mouseOffset = Vector2f::Zero();
}

Vector2f FreeFlyCamera::getSmoothMouseOffset()
{
  Vector2f offset(0., 0.);
  for (int i = 0; i < m_bufferSize; i++) {
    offset += m_offsetBuffer[i];
  }
  return offset;
}

void FreeFlyCamera::resetKeyStates() {
  m_keyStates[KEY_FORWARD] = false;
  m_keyStates[KEY_BACKWARD] = false;
  m_keyStates[KEY_LEFT] = false;
  m_keyStates[KEY_RIGHT] = false;
  m_keyStates[KEY_UP] = false;
  m_keyStates[KEY_DOWN] = false;
}