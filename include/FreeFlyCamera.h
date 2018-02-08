#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include <Eigen/Dense>
#include <Eigen/StdVector>

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class FreeFlyCamera
{
public:
  typedef enum {
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT,
    FORWARDLEFT,
    FORWARDRIGHT,
    BACKWARDLEFT,
    BACKWARDRIGHT,
    UP,
    DOWN,
    NONE
  } Direction;

  typedef enum {
    KEY_FORWARD,
    KEY_BACKWARD,
    KEY_LEFT,
    KEY_RIGHT,
    KEY_UP,
    KEY_DOWN,
  } Key;

  FreeFlyCamera();
  // Constructor with vectors
  FreeFlyCamera(const Eigen::Vector3f &position, const Eigen::Vector3f &direction, int width, int height);

  // Set position of camera
  void setPosition(const Eigen::Vector3f &position);
  // Set camera direction
  void setDirection(const Eigen::Vector3f &direction);
  // Set perspective constants (fovY : vertical field of view (in radian))
  void setPerspective(float fovY, float near, float far);
  // Set Viewport
  void setViewport(int width, int height);
  // Set speed
  void setSpeed(float speed);
  //Centre la caméra sur une bounding box
  void centerOnAABB(const Eigen::AlignedBox<float, 3> &bBox, const Eigen::Vector3f &dir = Eigen::Vector3f(0., 0., 0.));

  // Returns the view matrix
  const Eigen::Affine3f &viewMatrix() const;
  // Returns the projection matrix
  const Eigen::Matrix4f &projectionMatrix() const;

  Eigen::Vector3f direction();
  Eigen::Vector3f up();
  Eigen::Vector3f right();

  void update(float dt);

  void processKeyPress(Key key);
  void processKeyRelease(Key key);
  void processMousePress(int mouseX, int mouseY);
  void processMouseRelease();
  void processMouseMove(int mouseX, int mouseY);
  void processMouseScroll(float yoffset);
  void stopMovement();

  void setMouseOffsetBufferSize(size_t size);
private:
  void updateProjectionMatrix();
  void updateViewMatrix();

private:
  // FreeFlyCamera Perspective
  float m_width;
  float m_height;
  float m_near;
  float m_far;
  float m_fovy;

  // Matrices
  Eigen::Matrix4f m_ProjectionMatrix;
  Eigen::Affine3f m_viewMatrix;

  // FreeFlyCamera world coordinates
  Eigen::Vector3f m_position;
  Eigen::Vector3f m_worldUp = Eigen::Vector3f(0.f, 1.f, 0.f);
  Eigen::Vector3f m_direction;
  float m_yaw;
  float m_pitch;

  // State


  // FreeFlyCamera movements
  float m_speed = 0.01;
  float m_sensitivity = 0.0005;
  Direction m_move = NONE;

  // Mouse offset buffer
  void initOffsetBuffer();
  void updateOffsetBuffer();
  Eigen::Vector2f getSmoothMouseOffset();

  int m_bufferSize = 5;
  std::vector<Eigen::Vector2f> m_offsetBuffer;
  int m_offsetBufferEnd;

  Eigen::Vector2f m_mouseLastPos, m_mouseOffset;
};

#endif // CAMERA_H
