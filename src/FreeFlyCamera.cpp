#include <FreeFlyCamera.h>

#include <iostream>

#include <utils.h>

using namespace Eigen;

FreeFlyCamera::FreeFlyCamera()
  : m_position(Vector3f::Zero()), m_direction(Vector3f::UnitZ()), m_yaw(0), m_pitch(0), m_width(800), m_height(600)
  , m_fovy(M_PI / 3.f), m_near(0.1), m_far(50000)
{
  m_viewMatrix.setIdentity();
  setDirection(m_direction);

  updateProjectionMatrix();
  initOffsetBuffer();
}

FreeFlyCamera::FreeFlyCamera(const Eigen::Vector3f &position, const Eigen::Vector3f &direction, int width, int height)
{
  m_viewMatrix.setIdentity();

  m_position = position;
  setDirection(direction);

  m_yaw = 0;
  m_pitch = 0;

  m_width = width;
  m_height = height;

  m_fovy = M_PI / 3.f;
  m_near = 0.1;
  m_far = 50000.f;

  updateProjectionMatrix();
  initOffsetBuffer();
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

  m_yaw = asin(m_direction.z() / cos(m_pitch));
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
  float dist = m_speed * dt;

  switch (m_move) {
    case FORWARD:
      m_position += direction() * dist;
      break;
    case BACKWARD:
      m_position -= direction() * dist;
      break;
    case LEFT:
      m_position -= right() * dist;
      break;
    case RIGHT:
      m_position += right() * dist;
      break;
    case FORWARDLEFT:
      m_position += (- right() * INV_SQRT_TWO * dist + direction() * INV_SQRT_TWO * dist);
      break;
    case FORWARDRIGHT:
      m_position += (right() * INV_SQRT_TWO * dist + direction() * INV_SQRT_TWO * dist);
      break;
    case BACKWARDLEFT:
      m_position += (- right() * INV_SQRT_TWO * dist - direction() * INV_SQRT_TWO * dist);
      break;
    case BACKWARDRIGHT:
      m_position += (right() * INV_SQRT_TWO * dist - direction() * INV_SQRT_TWO * dist);
      break;
    case UP:
      m_position += m_worldUp * dist;
      break;
    case DOWN:
      m_position -= m_worldUp * dist;
      break;
    case NONE:
      break;
  }

  updateOffsetBuffer();
  Vector2f offset = getSmoothMouseOffset();

  if (offset != Vector2f::Zero()) {
    m_yaw   -= offset.x();
    m_pitch -= offset.y();

    if (radToDeg(m_pitch) > 89.0f)
      m_pitch = degToRad(89.0f);
    if (radToDeg(m_pitch) < -89.0f)
      m_pitch = degToRad(-89.0f);
  }

  updateViewMatrix();

  std::cout << "Pitch : " << radToDeg(m_pitch) << ", Yaw : " << radToDeg(m_yaw) << std::endl;

  m_mouseOffset = Vector2f::Zero();
}

void FreeFlyCamera::processKeyPress(Key key)
{
  switch (key)
  {
    case KEY_FORWARD:
      if (m_move == LEFT)
        m_move = FORWARDLEFT;
      else if (m_move == RIGHT)
        m_move = FORWARDRIGHT;
      else
        m_move = FORWARD;
      break;
    case KEY_BACKWARD:
      if (m_move == LEFT)
        m_move = BACKWARDLEFT;
      else if (m_move == RIGHT)
        m_move = BACKWARDRIGHT;
      else
        m_move = BACKWARD;
      break;
    case KEY_RIGHT:
      if (m_move == FORWARD)
        m_move = FORWARDRIGHT;
      else if (m_move == BACKWARD)
        m_move = BACKWARDRIGHT;
      else
        m_move = RIGHT;
      break;
    case KEY_LEFT:
      if (m_move == FORWARD)
        m_move = FORWARDLEFT;
      else if (m_move == BACKWARD)
        m_move = BACKWARDLEFT;
      else
        m_move = LEFT;
      break;
    case KEY_UP:
      m_move = UP;
      break;
    case KEY_DOWN:
      m_move = DOWN;
      break;
  }
}

void FreeFlyCamera::processKeyRelease(Key key)
{
  switch (key)
  {
    case KEY_FORWARD:
      if (m_move == FORWARD)
        m_move = NONE;
      else if (m_move == FORWARDLEFT)
        m_move = LEFT;
      else if (m_move == FORWARDRIGHT)
        m_move = RIGHT;
      break;
    case KEY_BACKWARD:
      if (m_move == BACKWARD)
        m_move = NONE;
      else if (m_move == BACKWARDLEFT)
        m_move = LEFT;
      else if (m_move == BACKWARDRIGHT)
        m_move = RIGHT;
      break;
    case KEY_RIGHT:
      if (m_move == RIGHT)
        m_move = NONE;
      else if (m_move == FORWARDRIGHT)
        m_move = FORWARD;
      else if (m_move == BACKWARDRIGHT)
        m_move = BACKWARD;
      break;
    case KEY_LEFT:
      if (m_move == LEFT)
        m_move = NONE;
      else if (m_move == FORWARDLEFT)
        m_move = FORWARD;
      else if (m_move == BACKWARDLEFT)
        m_move = BACKWARD;
      break;
    case KEY_UP:
      if (m_move == UP)
        m_move = NONE;
      break;
    case KEY_DOWN:
      if (m_move == DOWN)
        m_move = NONE;
      break;
  }
}

void FreeFlyCamera::processMousePress(int mouseX, int mouseY)
{
  m_mouseLastPos.x() = mouseX;
  m_mouseLastPos.y() = -mouseY;
}

void FreeFlyCamera::processMouseRelease()
{
}

void FreeFlyCamera::processMouseMove(int mouseX, int mouseY)
{
  Vector2f mousePos(mouseX, - mouseY);
  m_mouseOffset = mousePos - m_mouseLastPos;
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
  m_move = NONE;
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
  //Remplissage du buffer de mouvements de la souris
  m_offsetBuffer[m_offsetBufferEnd] = m_mouseOffset * m_sensitivity;
  m_offsetBufferEnd++;
  if (m_offsetBufferEnd == m_bufferSize) m_offsetBufferEnd = 0;
}

Vector2f FreeFlyCamera::getSmoothMouseOffset()
{
  Vector2f offset(0., 0.);
  for (int i = 0; i < m_bufferSize; i++) {
    offset += m_offsetBuffer[i];
  }
  return offset;
}

