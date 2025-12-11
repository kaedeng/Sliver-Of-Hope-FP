#include "ArcballCam.hpp"

#include <glm/gtc/matrix_transform.hpp>

CSCI441::ArcballCam::ArcballCam() : Camera() {
  mProjectionMatrix = glm::perspective(45.0f, 640.0f / 480.0f, 0.1f, 100.0f);
  mCameraTheta = 0.0f;
  mCameraRadius = 15.0f;
  mCameraPosition = glm::vec3(0.0f, 10.0f, 15.0f);
  mCameraLookAtPoint = glm::vec3(0.0f, -1.0f, -1.5f);
  mCameraUpVector = glm::vec3(0.0f, 1.0f, 0.0f);
  recomputeOrientation();
}

// Updates the camera's position and direction
void CSCI441::ArcballCam::recomputeOrientation() {
  // Prevent the camera from clipping through the ground plane
  if (mCameraPhi > glm::half_pi<float>() - 0.01f) {
    mCameraPhi = glm::half_pi<float>() - 0.01f;
  }
  if (mCameraPhi < 0.01f) {
    mCameraPhi = 0.01f;
  }

  // Calculate the new camera position based on the look-at point, radius, and
  // angles
  GLfloat camX = mCameraLookAtPoint.x +
                 mCameraRadius * sinf(mCameraPhi) * sinf(mCameraTheta);
  GLfloat camY = mCameraLookAtPoint.y + mCameraRadius * cosf(mCameraPhi);
  GLfloat camZ = mCameraLookAtPoint.z +
                 mCameraRadius * sinf(mCameraPhi) * cosf(mCameraTheta);

  mCameraPosition = glm::vec3(camX, camY, camZ);

  // Re-calculate the camera's direction vector
  mCameraDirection = glm::normalize(mCameraLookAtPoint - mCameraPosition);

  // Update the view matrix
  computeViewMatrix();
}

void CSCI441::ArcballCam::moveForward(GLfloat movementFactor) {
  mCameraRadius -= movementFactor;
  if (mCameraRadius < 0.1f) {
    mCameraRadius = 0.1f;
  }
  recomputeOrientation();
}

void CSCI441::ArcballCam::moveBackward(GLfloat movementFactor) {
  mCameraRadius += movementFactor;
  recomputeOrientation();
}
