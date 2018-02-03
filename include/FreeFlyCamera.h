#ifndef CAMERA_H
#define CAMERA_H

#include <memory>

#include <Eigen/Dense>
#include<Eigen/StdVector>
/*
// Default camera values
const float YAW        = -90.0f;
const float PITCH      =  0.0f;
const float SPEED      =  3.0f;
const float SENSITIVTY =  0.05f;
const float ZOOM       =  45.0f;

const float invsqrt2 = 0.7071;

// An abstract camera class that processes input and calculates the corresponding Eular Angles, Vectors and Matrices for use in OpenGL
class Camera
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

  // Camera Attributes
  Eigen::Vector3f m_position;
  Eigen::Vector3f m_front;
  Eigen::Vector3f m_up;
  Eigen::Vector3f m_right;
  Eigen::Vector3f m_worldUp;
  // Camera Perspective
  const float m_fr = 1.3807118e-04;
  const float m_fz = 0.1;
  float m_screenRatio;
  float m_screenWidth = 800.f;
  float m_screenHeight = 600.f;
  float m_zoom = 1.f;
  float m_near = 0.1; // TODO : Remove
  float m_far = 100;
  float m_tanHalfFovy;
  Eigen::Matrix4f m_projection;
  // Eular Angles
  float m_yaw;
  float m_pitch;
  // Camera options
  float m_movementSpeed;
  float m_mouseSensitivity;
  float m_fovy;
  Direction m_direction;

  // Constructor with vectors
  Camera(Eigen::Vector3f position = Eigen::Vector3f(0.0f, 0.0f, 0.0f), Eigen::Vector3f up = Eigen::Vector3f(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH);
  // Constructor with scalar values
  Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch);

  // Returns the view matrix calculated using Eular Angles and the LookAt Matrix
  Eigen::Matrix4f getViewMatrix() const;
  // Returns the projection matrix
  const Eigen::Matrix4f &getProjection() const;

  void setScreenSize(int width, int height);
  //Centre la caméra sur une bounding box
  void centerOnAABB(const Eigen::AlignedBox &bBox, const Eigen::Vector3f &dir = Eigen::Vector3f(0., 0., 0.));

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
  //Calcule la matrice de projection
  void calcProjection();
  // Calculates the front vector from the Camera's (updated) Eular Angles
  void updateCameraVectors();

  //Mouse offset buffer
  void initOffsetBuffer();
  void updateOffsetBuffer();
  Eigen::Vector2f getSmoothMouseOffset();

  int m_bufferSize = 5;
  std::vector<Eigen::Vector2f> m_offsetBuffer;
  int m_offsetBufferEnd;

  Eigen::Vector2f m_mouseLastPos, m_mouseOffset;
};
*/
#endif // CAMERA_H
