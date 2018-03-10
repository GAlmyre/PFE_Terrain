#ifndef TERRAINTINTIN_CAMERA_H
#define TERRAINTINTIN_CAMERA_H

#include <Eigen/Dense>

/* Base class for camera */
class Camera {
public:
  typedef enum { KEY_FORWARD = 0, KEY_BACKWARD, KEY_LEFT, KEY_RIGHT, KEY_UP, KEY_DOWN } Key;
  typedef enum { LEFT = 0, RIGHT, MIDDLE } Button;

public:
  Camera();

  // Set speed
  void setSpeed(float speed);
  float speed() const;

  // View and projection matrix getters
  virtual const Eigen::Affine3f &viewMatrix() const;
  virtual const Eigen::Matrix4f &projectionMatrix() const;

  // Get a ray from screen coordinates
  virtual void screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const = 0;

  // Update camera if needed
  virtual void update(float dt) = 0;

  // Handle user interaction
  virtual void processKeyPress(Key key) = 0;
  virtual void processKeyRelease(Key key) = 0;
  virtual void processMousePress(int mouseX, int mouseY) = 0;
  virtual void processMouseRelease() = 0;
  virtual void processMouseMove(int mouseX, int mouseY) = 0;
  virtual void processMouseScroll(float yoffset) = 0;
  virtual void stopMovement() = 0;

protected:
  // Projection and View matrices
  Eigen::Matrix4f m_ProjectionMatrix;
  Eigen::Affine3f m_viewMatrix;

  // Movements
  float m_speed = 0.01;
  float m_sensitivity = 0.0005;
};

/* Base class for perspective camera */
class PerspectiveCamera : public Camera {
public:
  PerspectiveCamera();
  PerspectiveCamera(int width, int height, float fovY = M_PI / 3.f, float near = 0.1, float far = 50000);
  // Set perspective constants (fovY : vertical field of view (in radian))
  void setPerspective(float fovY, float near, float far);
  // Set Viewport
  void setViewport(int width, int height);
  // Get Viewport
  Eigen::Vector2f viewport() const;

protected:
  void updateProjectionMatrix();

protected:
  // Perspective variables
  float m_width;
  float m_height;
  float m_near;
  float m_far;
  float m_fovy;
};

#endif //TERRAINTINTIN_CAMERA_H
