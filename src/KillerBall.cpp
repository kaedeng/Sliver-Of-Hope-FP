#include "KillerBall.h"
#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

KillerBall::KillerBall(GLuint shaderHandle, GLint mvpMtxLoc, GLint modelMtxLoc,
                       GLint normalMtxLoc, GLint materialColorLoc,
                       glm::vec3 position)
    : _shaderHandle(shaderHandle), _position(position), _radius(1.0f),
      _color(1.0f, 0.0f, 0.0f) // Bright Red
{
  _shaderLocs.mvpMtx = mvpMtxLoc;
  _shaderLocs.modelMtx = modelMtxLoc;
  _shaderLocs.normalMtx = normalMtxLoc;
  _shaderLocs.materialColor = materialColorLoc;
}

void KillerBall::draw(const glm::mat4 &viewMtx, const glm::mat4 &projMtx) {
  // Compute Model Matrix
  glm::mat4 modelMtx = glm::mat4(1.0f);
  modelMtx = glm::translate(modelMtx, _position);

  // Compute MVP and Normal Matrices
  glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
  glm::mat3 normalMtx = glm::transpose(glm::inverse(glm::mat3(modelMtx)));

  // Send Uniforms
  glProgramUniformMatrix4fv(_shaderHandle, _shaderLocs.mvpMtx, 1, GL_FALSE,
                            glm::value_ptr(mvpMtx));
  glProgramUniformMatrix4fv(_shaderHandle, _shaderLocs.modelMtx, 1, GL_FALSE,
                            glm::value_ptr(modelMtx));
  glProgramUniformMatrix3fv(_shaderHandle, _shaderLocs.normalMtx, 1, GL_FALSE,
                            glm::value_ptr(normalMtx));

  // Set color to Red
  glProgramUniform3fv(_shaderHandle, _shaderLocs.materialColor, 1,
                      glm::value_ptr(_color));

  // Draw Sphere
  CSCI441::drawSolidSphere(_radius, 16, 16);
}

void KillerBall::update(float deltaTime) {
  // Intentionally empty to keep the ball stationary
}

glm::vec3 KillerBall::getPosition() const { return _position; }

void KillerBall::setPosition(glm::vec3 position) {
  // No-op: Prevent the engine from pushing this enemy around during collision
  // resolution to ensure it remains a static hazard.
}

float KillerBall::getRadius() const { return _radius; }

void KillerBall::bounceOff(const glm::vec3 &otherPosition) {
  // No-op: Killer Ball does not bounce
}
