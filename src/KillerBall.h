#ifndef KILLERBALL_H
#define KILLERBALL_H

#include "Enemy.h"
#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class KillerBall final : public Enemy {
public:
  // Constructor matching the signature used in FPEngine.cpp
  KillerBall(GLuint shaderHandle, GLint mvpMtxLoc, GLint modelMtxLoc,
             GLint normalMtxLoc, GLint materialColorLoc, glm::vec3 position);

  // Draws the red sphere
  void draw(const glm::mat4 &viewMtx, const glm::mat4 &projMtx);

  // Update loop - moves along Bezier curve
  void update(float deltaTime);

  // Set up Bezier curve control points
  void setBezierControlPoints(const glm::vec3 &p0, const glm::vec3 &p1,
                              const glm::vec3 &p2, const glm::vec3 &p3);

  // Set the speed at which t parameter changes
  void setBezierSpeed(float speed);

  // Enemy Overrides
  glm::vec3 getPosition() const override;
  void setPosition(glm::vec3 position) override;
  float getRadius() const override;
  void bounceOff(const glm::vec3 &otherPosition) override;

private:
  GLuint _shaderHandle;
  struct ShaderLocs {
    GLint mvpMtx;
    GLint modelMtx;
    GLint normalMtx;
    GLint materialColor;
  } _shaderLocs;

  glm::vec3 _position;
  float _radius;
  glm::vec3 _color;

  // Bezier curve parameters
  glm::vec3 _bezierP0; // Start point
  glm::vec3 _bezierP1; // Control point 1
  glm::vec3 _bezierP2; // Control point 2
  glm::vec3 _bezierP3; // End point
  float _bezierSpeed;  // Units per second along the curve
  bool _bezierForward; // Direction of travel along curve

  // Arc-length parameterization
  static const int ARC_LENGTH_SAMPLES = 100;
  float _arcLengths[ARC_LENGTH_SAMPLES + 1]; // Cumulative arc lengths at each sample
  float _totalArcLength;                      // Total length of the curve
  float _currentArcLength;                    // Current position along arc length

  // Evaluate cubic Bezier curve at parameter t
  glm::vec3 _evaluateBezier(float t) const;

  // Build arc-length lookup table
  void _buildArcLengthTable();

  // Convert arc length to t parameter using lookup table
  float _arcLengthToT(float s) const;
};

#endif // KILLERBALL_H
