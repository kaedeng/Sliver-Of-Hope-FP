#include "Minion.h"

#include <CSCI441/OpenGLEngine.hpp>
#include <CSCI441/ShaderProgram.hpp>

Minion::Minion(GLuint shaderProgramHandle, GLint mvpMtxUniformLocation,
               GLint materialColorUniformLocation,
               GLint normalMtxUniformLocation, GLint modelMtxUniformLocation)
    : _shaderProgramHandle(shaderProgramHandle),
      _mvpMtxUniformLocation(mvpMtxUniformLocation),
      _materialColorUniformLocation(materialColorUniformLocation),
      _normalMtxUniformLocation(normalMtxUniformLocation),
      _modelMtxUniformLocation(modelMtxUniformLocation), _handBobOffset(0.0f) {
  _position = glm::vec3(8.0f, 0.0f, 8.0f);
  _direction = glm::vec3(0.0f, 0.0f, 1.0f);
  _angle = 0.0f;
}

Minion::~Minion() {}

// Renders the minion model by applying transformations and drawing each part
void Minion::draw(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) {

  // Apply minion's base transformations
  // modelMtx becomes the root of our hierarchy
  modelMtx = glm::translate(modelMtx, _position);
  modelMtx = glm::rotate(modelMtx, _angle, CSCI441::Y_AXIS);

  // Draw the body
  _drawBody(modelMtx, viewMtx, projMtx);

  // Draw the head and eyes
  _drawHead(modelMtx, viewMtx, projMtx);

  // Draw the left hand
  glm::mat4 leftHandModelMtx =
      glm::translate(modelMtx, glm::vec3(-1.5f, 1.0f, _handBobOffset));
  _drawHand(leftHandModelMtx, viewMtx, projMtx);

  // Draw the right hand
  glm::mat4 rightHandModelMtx =
      glm::translate(modelMtx, glm::vec3(1.5f, 1.0f, -_handBobOffset));
  _drawHand(rightHandModelMtx, viewMtx, projMtx);
}

// Updates the minion's state over time, such as animations
void Minion::update(float time) {
  // Calculate the vertical offset using a sine wave
  _handBobOffset = 1.0f * sin(time * 6.0f);
}

// Returns the minion's position
glm::vec3 Minion::getPosition() const { return _position; }

// Returns the minion's head position
glm::vec3 Minion::getHeadPosition() const {
  return _position + glm::vec3(0.0f, 4.0f, 0.0f);
}

// Sets the minion's position
void Minion::setPosition(glm::vec3 position) { _position = position; }

// Returns the minion's direction
glm::vec3 Minion::getDirection() const { return _direction; }

// Sets the minion's direction
void Minion::setDirection(glm::vec3 direction) { _direction = direction; }

// Returns the minion's angle
GLfloat Minion::getAngle() const { return _angle; }

// Sets the minion's angle
void Minion::setAngle(GLfloat angle) { _angle = angle; }

// Draws the minion's body
void Minion::_drawBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                       const glm::mat4 &projMtx) const {

  _setMatrices(modelMtx, viewMtx, projMtx);
  _setColor(glm::vec3(1.0f, 0.0f, 0.0f));
  CSCI441::drawSolidCone(1.0f, 4.0f, 16, 16);
}

// Draws the minion's head and eyes
void Minion::_drawHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                       const glm::mat4 &projMtx) const {
  // Draw Head
  glm::mat4 headModelMtx =
      glm::translate(modelMtx, glm::vec3(0.0f, 4.0f, 0.0f));

  headModelMtx = glm::scale(headModelMtx, glm::vec3(1.0f, 1.0f, 1.0f));

  _setMatrices(headModelMtx, viewMtx, projMtx);
  _setColor(glm::vec3(1.0f, 1.0f, 1.0f));
  CSCI441::drawSolidSphere(1.0f, 16, 16);

  // Draw Eyes (relative to Head)
  // Left Eye
  glm::mat4 leftEyeModelMtx =
      glm::translate(headModelMtx, glm::vec3(-0.383f, 0.0f, 0.924f));
  leftEyeModelMtx =
      glm::scale(leftEyeModelMtx, glm::vec3(0.125f, 0.125f, 0.125f));
  _drawEye(leftEyeModelMtx, viewMtx, projMtx);

  // Right Eye
  glm::mat4 rightEyeModelMtx =
      glm::translate(headModelMtx, glm::vec3(0.383f, 0.0f, 0.924f));
  rightEyeModelMtx =
      glm::scale(rightEyeModelMtx, glm::vec3(0.125f, 0.125f, 0.125f));
  _drawEye(rightEyeModelMtx, viewMtx, projMtx);

  // Draw Brows (relative to Head)
  // Left Brow
  glm::mat4 leftBrowModelMtx =
      glm::translate(headModelMtx, glm::vec3(-0.383f, 0.25f, 0.9f));
  leftBrowModelMtx = glm::rotate(leftBrowModelMtx, glm::radians(-45.0f),
                                 glm::vec3(0.0f, 0.0f, 1.0f));
  leftBrowModelMtx = glm::scale(leftBrowModelMtx, glm::vec3(0.75f, 0.1f, 0.1f));
  _drawBrow(leftBrowModelMtx, viewMtx, projMtx);

  // Right Brow
  glm::mat4 rightBrowModelMtx =
      glm::translate(headModelMtx, glm::vec3(0.383f, 0.25f, 0.9f));
  rightBrowModelMtx = glm::rotate(rightBrowModelMtx, glm::radians(45.0f),
                                  glm::vec3(0.0f, 0.0f, 1.0f));
  rightBrowModelMtx =
      glm::scale(rightBrowModelMtx, glm::vec3(0.75f, 0.1f, 0.1f));
  _drawBrow(rightBrowModelMtx, viewMtx, projMtx);
}

// Draws an eye
void Minion::_drawEye(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                      const glm::mat4 &projMtx) const {

  _setMatrices(modelMtx, viewMtx, projMtx);
  _setColor(glm::vec3(1.0f, 0.0f, 0.0f));
  CSCI441::drawSolidSphere(1.0f, 16, 16);
}

// Draws a brow
void Minion::_drawBrow(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                       const glm::mat4 &projMtx) const {

  _setMatrices(modelMtx, viewMtx, projMtx);
  _setColor(glm::vec3(0.0f, 0.0f, 0.0f));
  CSCI441::drawSolidCube(1.0f);
}

// Draws a hand
void Minion::_drawHand(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                       const glm::mat4 &projMtx) const {

  modelMtx = glm::scale(modelMtx, glm::vec3(0.5f, 0.5f, 0.5f));

  _setMatrices(modelMtx, viewMtx, projMtx);
  _setColor(glm::vec3(1.0f, 1.0f, 1.0f));
  CSCI441::drawSolidSphere(1.0f, 16, 16);
}

// Sends the MVP and normal matrices to the shader
void Minion::_setMatrices(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) const {
  // Precompute the MVP matrix
  glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
  // Send MVP matrix to the shader
  glProgramUniformMatrix4fv(_shaderProgramHandle, _mvpMtxUniformLocation, 1,
                            GL_FALSE, &mvpMtx[0][0]);

  // Compute and send the Normal matrix
  glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
  glProgramUniformMatrix3fv(_shaderProgramHandle, _normalMtxUniformLocation, 1,
                            GL_FALSE, &normalMtx[0][0]);
  glProgramUniformMatrix4fv(_shaderProgramHandle, _modelMtxUniformLocation, 1,
                            GL_FALSE, &modelMtx[0][0]);
}

// _setColor
// Sends the material color to the shader
void Minion::_setColor(glm::vec3 color) const {
  glProgramUniform3fv(_shaderProgramHandle, _materialColorUniformLocation, 1,
                      &color[0]);
}
