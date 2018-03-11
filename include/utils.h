#ifndef TERRAINTINTIN_UTILS_H
#define TERRAINTINTIN_UTILS_H

#include <cmath>
#include <Eigen/Geometry>

#undef M_PI

#define M_PI         3.14159265358979323846f
#define INV_PI       0.31830988618379067154f
#define INV_TWOPI    0.15915494309189533577f
#define INV_FOURPI   0.07957747154594766788f
#define SQRT_TWO     1.41421356237309504880f
#define INV_SQRT_TWO 0.70710678118654752440f

//// Convert radians to degrees
inline float radToDeg(float value) { return value * (180.0f / M_PI); }

/// Convert degrees to radians
inline float degToRad(float value) { return value * (M_PI / 180.0f); }

inline float getY(const Eigen::Hyperplane<float, 3> &plane, float x, float z) {
  const auto &c = plane.coeffs();
  return - (c[0] * x + c[2]* z + c[3]) / c[1];
}

#endif //TERRAINTINTIN_UTILS_H
