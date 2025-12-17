#ifndef KILLERBALL_H
#define KILLERBALL_H

#include "Enemy.h"
#include <glad/gl.h>
#include <glm/glm.hpp>

class KillerBall final : public Enemy {
public:
  // Constructor matching the signature used in FPEngine.cpp
  KillerBall(GLuint shaderHandle, GLint mvpMtxLoc, GLint modelMtxLoc,
             GLint normalMtxLoc, GLint materialColorLoc, glm::vec3 position);

  // Draws the red sphere
  void draw(const glm::mat4 &viewMtx, const glm::mat4 &projMtx);

  // Update loop (no-op for stationary object)
  void update(float deltaTime);

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
};

#endif // KILLERBALL_H
