#include <FreeFlyCamera.h>

#include <iostream>
#include <algorithm>

#include <utils.h>

using namespace Eigen;

FreeFlyCamera::FreeFlyCamera()
  : PerspectiveCamera(), m_position(Vector3f::Zero()), m_direction(Vector3f::UnitZ()), m_yaw(0), m_pitch(0),
    m_mouseOffset(0.f, 0.f), m_rotating(false), m_upOffset(0), m_grabbed(false)
{
  setDirection(m_direction);

  initOffsetBuffer();
  resetKeyStates();
}

FreeFlyCamera::FreeFlyCamera(const Eigen::Vector3f &position, const Eigen::Vector3f &direction, int width, int height)
  : PerspectiveCamera(width, height), m_position(position), m_yaw(0), m_pitch(0),
    m_mouseOffset(0.f, 0.f), m_rotating(false), m_upOffset(0), m_grabbed(false)
{
  setDirection(direction);

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


  if (m_grabbed)
    m_viewMatrix.translation() = - (m_viewMatrix.linear() * (m_position + m_upOffset * m_worldUp));
  else
    m_viewMatrix.translation() = - (m_viewMatrix.linear() * m_position);
}

void FreeFlyCamera::screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const {
  orig = m_position;
  if (grabedToGround())
    orig += m_upOffset * m_worldUp;

  Vector3f localDir = Vector3f( ((2.0 * p[0] / m_width) - 1.0) * tan(m_fovy/2.0) * m_width / m_height,
                                ((2.0 * (m_height - p[1]) / m_height) - 1.0) * tan(m_fovy/2.0),
                                1.0 );

  dir = (localDir.x() * right() + localDir.y() * up() + localDir.z() * direction()).normalized();
}

Eigen::Vector3f FreeFlyCamera::position() const {
  return m_position;
}

Eigen::Vector3f FreeFlyCamera::direction() const {
  return -m_viewMatrix.linear().row(2);
}

Eigen::Vector3f FreeFlyCamera::up() const {
  return m_viewMatrix.linear().row(1);
}

Eigen::Vector3f FreeFlyCamera::right() const {
  return m_viewMatrix.linear().row(0);
}

void FreeFlyCamera::update(float dt)
{
  // Update Position
  Vector3f dir = Vector3f::Zero();
  if (m_grabbed) {
    if (m_keyStates[KEY_FORWARD])  dir += Vector3f(direction().x(), 0.f, direction().z());
    if (m_keyStates[KEY_BACKWARD]) dir -= Vector3f(direction().x(), 0.f, direction().z());
    if (m_keyStates[KEY_RIGHT])    dir += Vector3f(right().x(), 0.f, right().z());
    if (m_keyStates[KEY_LEFT])     dir -= Vector3f(right().x(), 0.f, right().z());
  } else {
    if (m_keyStates[KEY_FORWARD])  dir += direction();
    if (m_keyStates[KEY_BACKWARD]) dir -= direction();
    if (m_keyStates[KEY_RIGHT])    dir += right();
    if (m_keyStates[KEY_LEFT])     dir -= right();
  }
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

void FreeFlyCamera::setUpOffset(float offset) {
  m_upOffset = offset;
}

bool FreeFlyCamera::grabedToGround() const {
  return m_grabbed;
}

void FreeFlyCamera::gradToGround(bool grab) {
  m_grabbed = grab;
}

void FreeFlyCamera::processMouseScroll(float yoffset) {

}
