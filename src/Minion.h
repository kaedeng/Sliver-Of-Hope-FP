#ifndef MINION_H
#define MINION_H

#include <glad/gl.h>

#include <glm/glm.hpp>

#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Minion {
public:
  Minion(GLuint shaderProgramHandle, GLint mvpMtxUniformLocation,
         GLint materialColorUniformLocation, GLint normalMtxUniformLocation,
         GLint modelMtxUniformLocation);
  ~Minion();

  // Updates the minion's state.
  void update(float time);

  // Draws the minion.
  void draw(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx);

  // Gets the minion's position.
  glm::vec3 getPosition() const;

  // Gets the minion's head position.
  glm::vec3 getHeadPosition() const;

  // Sets the minion's position.
  void setPosition(glm::vec3 position);

  // Gets the minion's direction.
  glm::vec3 getDirection() const;

  // Sets the minion's direction.
  void setDirection(glm::vec3 direction);

  // Gets the minion's angle.
  GLfloat getAngle() const;

  // Sets the minion's angle.
  void setAngle(GLfloat angle);

private:
  glm::vec3 _position;
  glm::vec3 _direction;
  GLfloat _angle;
  GLfloat _handBobOffset;

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
