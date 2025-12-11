#ifndef ARCBALL_CAM_H
#define ARCBALL_CAM_H

#include <CSCI441/Camera.hpp>

namespace CSCI441 {

class ArcballCam : public Camera {
public:
  ArcballCam();

  // Updates the camera's position and direction
  void recomputeOrientation() override;

  void moveForward(GLfloat speed) override;
  void moveBackward(GLfloat speed) override;
};

} // namespace CSCI441

#endif
