#include "Camera.h"

using namespace Eigen;

Camera::Camera() {
  m_ProjectionMatrix = Eigen::Matrix4f::Identity();
  m_viewMatrix = Eigen::Affine3f::Identity();
}

const Eigen::Matrix4f &Camera::projectionMatrix() const {
  return m_ProjectionMatrix;
}

const Eigen::Affine3f &Camera::viewMatrix() const {
  return m_viewMatrix;
}

PerspectiveCamera::PerspectiveCamera()
  : Camera(), m_width(800), m_height(600), m_fovy(M_PI / 3.f), m_near(0.1), m_far(50000)
{
  updateProjectionMatrix();
}

PerspectiveCamera::PerspectiveCamera(int width, int height, float fovY, float near, float far)
  : Camera(), m_width(width), m_height(height), m_fovy(fovY), m_near(near), m_far(far)
{
  updateProjectionMatrix();
}

void PerspectiveCamera::setPerspective(float fovY, float near, float far) {
  m_fovy = fovY;
  m_near = near;
  m_far = far;
  updateProjectionMatrix();
}

void PerspectiveCamera::setViewport(int width, int height) {
  m_width = width;
  m_height = height;
  updateProjectionMatrix();
}

Eigen::Vector2f PerspectiveCamera::viewport() const {
  return Vector2f((float) m_width, (float) m_height);
}

void PerspectiveCamera::updateProjectionMatrix() {
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
