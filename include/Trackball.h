#ifndef TERRAINTINTIN_TRACKBALL_H
#define TERRAINTINTIN_TRACKBALL_H

#include "Camera.h"

class Trackball : public PerspectiveCamera
{
public:
  enum State {
    Idle, Rotating, Translating
  };


public:
  Trackball();

  /// \brief The center of the view. Trackball rotate around this point.
  const Eigen::Vector3f& sceneCenter() const;

  /// \brief The distance between `center` and the camera.
  float sceneDistance() const;

  /// \brief The radius of a circle centered on a object plane that define the fov.
  float sceneRadius() const;

  /// \brief The orientation of the camera.
  const Eigen::Quaternionf& sceneOrientation() const;

  Eigen::Matrix4f computeViewMatrix() const;

  void setSceneCenter(const Eigen::Vector3f& scnCenter);
  void setSceneDistance(float scnDistance);
  void setSceneRadius(float scnRadius);
  void setSceneOrientation(const Eigen::Quaternionf& scnOrientation);

  State state() const;
  bool isIdle() const;

  /// \brief Rotate around `center`.
  void rotate(const Eigen::Quaternionf& rot);
  bool isRotating() const;
  void startRotation(const Eigen::Vector2f& scrPos);
  void dragRotate(const Eigen::Vector2f& scrPos);
  void cancelRotation();
  void endRotation();

  /// \brief Translate in the view plane.
  bool isTranslating() const;
  void startTranslation(const Eigen::Vector2f& scrPos);
  void dragTranslate(const Eigen::Vector2f& scrPos);
  void cancelTranslation();
  void endTranslation();

  /// \brief Move camera forward or backward without changing fov. (So it
  /// changes radius.)
  void zoom(float factor);

  /// \brief Grow or shrink radius, changing the fov.
  void grow(float factor);

  /// \brief Do a dolly zoom: move the camera and change the fov so that the
  /// subject stay in the same frame.
  void dollyZoom(float factor);

  void update(float dt) override;

  void processKeyPress(Key key);

  virtual void screenPosToRay(const Eigen::Vector2i &p, Eigen::Vector3f &orig, Eigen::Vector3f &dir) const;

  void processKeyRelease(Key key) override;
  void processMousePress(int mouseX, int mouseY);
  void processMouseRelease();
  void processMouseMove(int mouseX, int mouseY);
  void processMouseScroll(float yoffset);
  void stopMovement();

  EIGEN_MAKE_ALIGNED_OPERATOR_NEW
private:
  Eigen::Vector2f normFromScr(const Eigen::Vector2f& scrPos) const;

  Eigen::Quaternionf computeRotation(const Eigen::Vector2f& scrPos) const;
  Eigen::Vector3f computeTranslation(const Eigen::Vector2f& scrPos) const;

private:
  Eigen::Vector3f _scnCenter;
  float _scnDistance;
  float _scnRadius;
  Eigen::Quaternionf _scnOrientation;

  State _state;
  Eigen::Vector2f _scrMouseInit;
  Eigen::Quaternionf _scnOrientInit;
  Eigen::Vector3f _scnCenterInit;
};

#endif //TERRAINTINTIN_TRACKBALL_H
