//
// Created by james on 10/31/2025.
//
#include "Wilfred.h"

#include <glm/gtc/matrix_transform.hpp>

#include <CSCI441/OpenGLUtils.hpp>
#include <CSCI441/objects.hpp>

Wilfred::Wilfred(const GLuint shaderProgramHandle,
                 const GLint mvpMtxUniformLocation,
                 const GLint normalMtxUniformLocation,
                 const GLint materialColorUniformLocation,
                 const GLint modelMtxUniformLocation)
    : _shaderProgramHandle(0), _shaderProgramUniformLocations({-1, -1, -1, -1}),
      _halfPi(s_PI_OVER_2), _rotationAngle(0.0f),
      _colorHead({0.9f, 0.875f, 0.627f}), _scaleHead({1.0f, 1.0f, 1.0f}),
      _colorBody({0.4f, 0.659f, 0.412f}), _colorCane({0.46f,0.412f,0.208}), _scaleBody({0.9f, 2.0f, 1.0f}),
      _scaleArm({1.0f, 1.0f, 3.0f}), _scaleHand({1.0f, 1.0f, 0.333f}),
_moveSpeed(5.0f),  _radius(0.5f),
      _alive(true),
      _falling(false),
      _verticalVelocity(0.0f),
      _animOffset(0.0f), _position({4.0f, 0.0f, 0.0f}), direction(0.0005f) {
    _headingVector = glm::normalize(glm::vec3(sin(0), 0.0f, cos(0)));
  setProgramUniformLocations(
      shaderProgramHandle, mvpMtxUniformLocation, normalMtxUniformLocation,
      materialColorUniformLocation, modelMtxUniformLocation);
}

void Wilfred::setProgramUniformLocations(
    const GLuint shaderProgramHandle, const GLint mvpMtxUniformLocation,
    const GLint normalMtxUniformLocation,
    const GLint materialColorUniformLocation,
    const GLint modelMtxUniformLocation) {
  _shaderProgramHandle = shaderProgramHandle;
  _shaderProgramUniformLocations = {
      mvpMtxUniformLocation, normalMtxUniformLocation,
      materialColorUniformLocation, modelMtxUniformLocation};
}

void Wilfred::drawWilfred(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) {
  modelMtx = glm::translate(modelMtx, _position);
  // scale that bitch
  modelMtx = glm::scale(modelMtx, {7, 7, 7});
  // rotate the character to make him upright
  modelMtx = glm::rotate(modelMtx, 1.5708f, CSCI441::X_AXIS);
  modelMtx = glm::rotate(modelMtx, -1.5708f, CSCI441::Z_AXIS);
  modelMtx = glm::rotate(modelMtx, -_halfPi, CSCI441::Y_AXIS);
  modelMtx = glm::rotate(modelMtx, _halfPi, CSCI441::Z_AXIS);

  // rotation ability
  glm::vec3 headPivot =
      glm::vec3(0.0f, 0.0f, 0.0f); // center of the head in model space
  modelMtx = glm::translate(modelMtx, headPivot);
  modelMtx = glm::rotate(modelMtx, _rotationAngle, CSCI441::Y_AXIS);
  modelMtx = glm::translate(modelMtx, -headPivot);

  _drawBrosHead(modelMtx, viewMtx, projMtx); // the head of our character
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, _animOffset, 0.0f));
  _drawBrosUpperBody(modelMtx, viewMtx, projMtx); // character's upper body
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, -_animOffset, 0.0f));
  _drawBrosLowerBody(modelMtx, viewMtx, projMtx); // character's lower body
}

void Wilfred::_drawBrosHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                            const glm::mat4 &projMtx) {
  modelMtx = glm::scale(modelMtx, _scaleHead);
  // modelMtx = glm::rotate(modelMtx,_rotationAngle, CSCI441::Y_AXIS); //
  // turning

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorHead[0]);

  CSCI441::drawSolidSphere(0.1, 10, 10);
}

void Wilfred::_drawBrosUpperBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                                 const glm::mat4 &projMtx) const {
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, -0.13f, -0.13f));
  modelMtx = glm::rotate(modelMtx, 0.785398f, CSCI441::X_AXIS);
  modelMtx = glm::scale(modelMtx, glm::vec3(1.0f, 1.0f, 1.0f));

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorBody[0]);

  CSCI441::drawSolidCube(0.2);

  // now draw arm in relation to upper body
  _drawBrosArm(modelMtx, viewMtx, projMtx);
}

void Wilfred::_drawBrosArm(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                           const glm::mat4 &projMtx) const {
  modelMtx = glm::translate(modelMtx, glm::vec3(0.125f, 0.0f, 0.1f));
  modelMtx = glm::rotate(modelMtx, -0.3f, CSCI441::X_AXIS);
  modelMtx = glm::scale(modelMtx, _scaleArm);

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorBody[0]);

  CSCI441::drawSolidCube(0.05);

  // now draw the hand
  _drawBrosHand(modelMtx, viewMtx, projMtx);
}

void Wilfred::_drawBrosHand(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                            const glm::mat4 &projMtx) const {
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.0f, 0.03f));

  glm::vec3 handPivot = glm::vec3(0.0f, 0.0f, 0.0f);
  modelMtx = glm::translate(modelMtx, handPivot);
  modelMtx = glm::translate(modelMtx, -handPivot);
  modelMtx = glm::scale(modelMtx, _scaleHand); // fix odd scaling

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorBody[0]);

  CSCI441::drawSolidSphere(0.025, 10, 10);

  // now draw the cane
  _drawBrosCane(modelMtx, viewMtx, projMtx);
}

void Wilfred::_drawBrosCane(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                            const glm::mat4 &projMtx) const {
  modelMtx =
      glm::rotate(modelMtx, 1.085398f,
                  CSCI441::X_AXIS); // reset rotation so it starts facing down
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, 0.0f, 0.125f));
  modelMtx =
      glm::scale(modelMtx, glm::vec3(1.0f, 1.0f, 11.0f)); // fix odd scaling

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorCane[0]);

  CSCI441::drawSolidCube(0.025);
}

void Wilfred::_drawBrosLowerBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                                 const glm::mat4 &projMtx) const {
  modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, -0.35f, -0.16f));
  // modelMtx = glm::rotate(modelMtx,_rotationAngle, CSCI441::Y_AXIS); //
  // turning
  modelMtx = glm::scale(modelMtx, _scaleBody);

  _computeAndSendMatrixUniforms(modelMtx, viewMtx, projMtx);

  glProgramUniform3fv(_shaderProgramHandle,
                      _shaderProgramUniformLocations.materialColor, 1,
                      &_colorBody[0]);

  CSCI441::drawSolidCube(0.2);
}

void Wilfred::_animateBro() {
  if (_animOffset > 0.03f) { // rotation too far forward
    direction = -direction;
  } else if (_animOffset < -0.03f) { // rotation too far backward
    direction = -direction;
  }
  _animOffset += direction;
}

// Returns the hero's position
glm::vec3 Wilfred::getPosition() const { return _position; }

// Sets the hero's position
void Wilfred::setPosition(glm::vec3 position) { _position = position; }

// Sets the hero's angle
void Wilfred::setAngle(GLfloat angle) { _rotationAngle = angle; }

GLfloat Wilfred::getAngle() { return _rotationAngle; }

void Wilfred::setHeading(const glm::vec3& heading) {
    _headingVector = glm::normalize(heading);
}

void Wilfred::update(float deltaTime, const glm::vec3& heroPosition, float turnSpeed) {
    if (!_alive || _falling) {
        // if falling, apply gravity so the goomba can perish...
        if (_falling) {
            const float gravity = -20.0f;
            _verticalVelocity += gravity * deltaTime;
            _position.y += _verticalVelocity * deltaTime;
        }
        return;
    }

    // move along heading
    _position += _headingVector * _moveSpeed * deltaTime;

    // calculate vector from enemy to hero
    glm::vec3 toHero = heroPosition - _position;
    toHero.y = 0.0f; // Only turn in horizontal plane

    if (glm::length(toHero) > 0.01f) {
        glm::vec3 desiredHeading = glm::normalize(toHero);

        // angle between current heading and new heading
        float currentAngle = atan2(_headingVector.x, _headingVector.z);
        float desiredAngle = atan2(desiredHeading.x, desiredHeading.z);

        // shortest angular difference
        float angleDiff = desiredAngle - currentAngle;

        // normalize
        while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

        // one step towards the hero
        float maxTurn = turnSpeed * deltaTime;
        float turnAmount = glm::clamp(angleDiff, -maxTurn, maxTurn);

        float newAngle = currentAngle + turnAmount;
        _rotationAngle = newAngle;
        _headingVector = glm::normalize(glm::vec3(sin(newAngle), 0.0f, cos(newAngle)));
    }
}

void Wilfred::_computeAndSendMatrixUniforms(const glm::mat4 &modelMtx,
                                            const glm::mat4 &viewMtx,
                                            const glm::mat4 &projMtx) const {
  // precompute the Model-View-Projection matrix on the CPU
  glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
  // then send it to the shader on the GPU to apply to every vertex
  glProgramUniformMatrix4fv(_shaderProgramHandle,
                            _shaderProgramUniformLocations.mvpMtx, 1, GL_FALSE,
                            &mvpMtx[0][0]);

  glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
  glProgramUniformMatrix3fv(_shaderProgramHandle,
                            _shaderProgramUniformLocations.normalMtx, 1,
                            GL_FALSE, &normalMtx[0][0]);

  glProgramUniformMatrix4fv(_shaderProgramHandle,
                            _shaderProgramUniformLocations.modelMtx, 1,
                            GL_FALSE, &modelMtx[0][0]);
}
