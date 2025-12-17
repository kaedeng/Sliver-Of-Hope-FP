#ifndef FARINA_H
#define FARINA_H

#include "Enemy.h"

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Farina : public Enemy {
public:
  Farina(GLuint shaderProgramHandle, GLint mvpMtxUniformLocation,
         GLint materialColorUniformLocation, GLint normalMtxUniformLocation,
         GLint modelMtxUniformLocation);
  ~Farina();

  // Updates Farina's state.
  void update(float time, const glm::vec3 &heroPosition, float turnSpeed);

  // Draws Farina
  void draw(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx);

  // Causes Farina to bounce away from a collision point
  void bounceOff(const glm::vec3 &otherPosition) override;

  // Get & set Farina's position.
  glm::vec3 getPosition() const override;
  void setPosition(glm::vec3 position) override;

  // Get & set Farina's heading.
  glm::vec3 getHeading() const;
  void setHeading(glm::vec3 heading);

  // Get & set Farina's angle.
  GLfloat getAngle() const;
  void setAngle(GLfloat angle);

  // Get Farina's radius.
  float getRadius() const override;

private:
  glm::vec3 _position;
  glm::vec3 _heading;
  GLfloat _angle;

  float _moveSpeed;
  float _radius;
  bool _alive;
  bool _falling;
  float _verticalVelocity;

  GLfloat _handBobOffset;
  float _animPhase;

  GLuint _shaderProgramHandle;
  GLint _mvpMtxUniformLocation;
  GLint _materialColorUniformLocation;
  GLint _normalMtxUniformLocation;
  GLint _modelMtxUniformLocation;

  // Draws the main body of the minion.
  void _drawBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                 const glm::mat4 &projMtx) const;

  // Draws the head and eyes relative to the minion's root.
  void _drawHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                 const glm::mat4 &projMtx) const;

  // Draws an eye.
  void _drawEye(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                const glm::mat4 &projMtx) const;

  // Draws a brow
  void _drawBrow(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                 const glm::mat4 &projMtx) const;

  // Draws a hand.
  void _drawHand(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                 const glm::mat4 &projMtx) const;

  // Helper function to send matrix uniforms to the shader.
  void _setMatrices(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                    const glm::mat4 &projMtx) const;

  // Helper function to send the material color uniform to the shader.
  void _setColor(glm::vec3 color) const;
};

#endif // MINION_H
