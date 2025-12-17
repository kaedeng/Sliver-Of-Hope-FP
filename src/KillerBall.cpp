#include "KillerBall.h"
#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

KillerBall::KillerBall(GLuint shaderHandle, GLint mvpMtxLoc, GLint modelMtxLoc,
                       GLint normalMtxLoc, GLint materialColorLoc,
                       glm::vec3 position)
    : _shaderHandle(shaderHandle), _position(position), _radius(3.0f),
      _color(1.0f, 0.0f, 0.0f), // Bright Red
      _bezierP0(position), _bezierP1(position), _bezierP2(position),
      _bezierP3(position), _bezierSpeed(10.0f), _bezierForward(true),
      _totalArcLength(0.0f), _currentArcLength(0.0f), _arcLengths{} {
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
  // Update arc length position 
  if (_bezierForward) {
    _currentArcLength += _bezierSpeed * deltaTime;
    if (_currentArcLength >= _totalArcLength) {
      _currentArcLength = _totalArcLength;
      _bezierForward = false; // Reverse direction
    }
  } else {
    _currentArcLength -= _bezierSpeed * deltaTime;
    if (_currentArcLength <= 0.0f) {
      _currentArcLength = 0.0f;
      _bezierForward = true; // Reverse direction
    }
  }

  // Convert arc length to t parameter and evaluate curve
  float t = _arcLengthToT(_currentArcLength);
  _position = _evaluateBezier(t);
}

void KillerBall::setBezierControlPoints(const glm::vec3 &p0, const glm::vec3 &p1,
                                        const glm::vec3 &p2, const glm::vec3 &p3) {
  _bezierP0 = p0;
  _bezierP1 = p1;
  _bezierP2 = p2;
  _bezierP3 = p3;
  _currentArcLength = 0.0f;
  _buildArcLengthTable();
  _position = _evaluateBezier(0.0f);
}

void KillerBall::setBezierSpeed(float speed) { _bezierSpeed = speed; }

glm::vec3 KillerBall::_evaluateBezier(float t) const {
  // Cubic Bezier curve: B(t) = (1-t)^3*P0 + 3*(1-t)^2*t*P1 + 3*(1-t)*t^2*P2 + t^3*P3
  float oneMinusT = 1.0f - t;
  float oneMinusT2 = oneMinusT * oneMinusT;
  float oneMinusT3 = oneMinusT2 * oneMinusT;
  float t2 = t * t;
  float t3 = t2 * t;

  return oneMinusT3 * _bezierP0 + 3.0f * oneMinusT2 * t * _bezierP1 +
         3.0f * oneMinusT * t2 * _bezierP2 + t3 * _bezierP3;
}

void KillerBall::_buildArcLengthTable() {
  // Build cumulative arc length table by sampling the curve
  _arcLengths[0] = 0.0f;
  glm::vec3 prevPoint = _evaluateBezier(0.0f);

  for (int i = 1; i <= ARC_LENGTH_SAMPLES; ++i) {
    float t = static_cast<float>(i) / static_cast<float>(ARC_LENGTH_SAMPLES);
    glm::vec3 currentPoint = _evaluateBezier(t);
    float segmentLength = glm::length(currentPoint - prevPoint);
    _arcLengths[i] = _arcLengths[i - 1] + segmentLength;
    prevPoint = currentPoint;
  }

  _totalArcLength = _arcLengths[ARC_LENGTH_SAMPLES];
}

float KillerBall::_arcLengthToT(float s) const {
  // Binary search to find the t value corresponding to arc length s
  if (s <= 0.0f)
    return 0.0f;
  if (s >= _totalArcLength)
    return 1.0f;

  // Find the segment containing s
  int low = 0;
  int high = ARC_LENGTH_SAMPLES;

  while (low < high) {
    int mid = (low + high) / 2;
    if (_arcLengths[mid] < s) {
      low = mid + 1;
    } else {
      high = mid;
    }
  }

  // Interpolate within the segment
  int segmentIndex = (low > 0) ? low - 1 : 0;
  float segmentStart = _arcLengths[segmentIndex];
  float segmentEnd = _arcLengths[segmentIndex + 1];
  float segmentFraction = (s - segmentStart) / (segmentEnd - segmentStart);

  float tStart = static_cast<float>(segmentIndex) / static_cast<float>(ARC_LENGTH_SAMPLES);
  float tEnd = static_cast<float>(segmentIndex + 1) / static_cast<float>(ARC_LENGTH_SAMPLES);

  return tStart + segmentFraction * (tEnd - tStart);
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
