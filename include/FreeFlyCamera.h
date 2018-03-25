#ifndef CAMERA_H
#define CAMERA_H

#include <memory>
#include <map>
#include <Eigen/StdVector>

#include "Camera.h"

// A camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class FreeFlyCamera : public PerspectiveCamera
{
public:
  FreeFlyCamera();
  FreeFlyCamera(const Eigen::Vector3f &position, const Eigen::Vector3f &direction, int width, int height);

  // Set position of camera
  void setPosition(const Eigen::Vector3f &position);
  // Set camera direction
  void setDirection(const Eigen::Vector3f &direction);

  // Throw a ray through screen
  void screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const override;

  bool grabedToGround() const;
  void gradToGround(bool grab);
  void setUpOffset(float offset);

  Eigen::Vector3f position() const;
  Eigen::Vector3f direction() const;
  Eigen::Vector3f up() const;
  Eigen::Vector3f right() const;

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
  void updateViewMatrix();

  // Mouse offset buffer
  void initOffsetBuffer();
  void updateOffsetBuffer();
  Eigen::Vector2f getSmoothMouseOffset();

  void resetKeyStates();
private:
  // World coordinates
  Eigen::Vector3f m_position;
  Eigen::Vector3f m_worldUp = Eigen::Vector3f(0.f, 1.f, 0.f);
  Eigen::Vector3f m_direction;
  float m_yaw;
  float m_pitch;

  // Movements
  std::map<Key, bool> m_keyStates;
  bool m_rotating;
  float m_upOffset;
  bool m_grabbed;

  // Mouse Offsets
  size_t m_bufferSize = 5;
  std::vector<Eigen::Vector2f> m_offsetBuffer;
  int m_offsetBufferEnd;
  Eigen::Vector2f m_mouseLastPos, m_mouseOffset;
};

#endif // CAMERA_H
