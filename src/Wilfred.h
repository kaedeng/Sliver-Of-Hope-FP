//
// Created by james on 10/31/2025.
//

#ifndef A3IMSOTIRED_WILFRED_H
#define A3IMSOTIRED_WILFRED_H

#include <glad/gl.h>

#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp>

class Wilfred {
public:
  /// \desc creates a simple plane that gives the appearance of flight
  /// \param shaderProgramHandle shader program handle that the plane should be
  /// drawn using
  /// \param mvpMtxUniformLocation uniform location for the full precomputed MVP
  /// matrix
  /// \param normalMtxUniformLocation uniform location for the precomputed
  /// Normal matrix
  /// \param materialColorUniformLocation uniform location for the material
  /// diffuse color
  /// \param modelMtxUniformLocation uniform location for the precomputed model
  /// matrix
  Wilfred(GLuint shaderProgramHandle, GLint mvpMtxUniformLocation,
          GLint normalMtxUniformLocation, GLint materialColorUniformLocation,
          GLint modelMtxUniformLocation);

  /// draws the model plane for a given MVP (modelMtx) and camera (viewMtx,
  /// projMtx) matrices
  void drawWilfred(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                   const glm::mat4 &projMtx);

  /// \param shaderProgramHandle shader program handle that the plane should be
  /// drawn using
  /// \param mvpMtxUniformLocation uniform location for the full precomputed MVP
  /// matrix
  /// \param normalMtxUniformLocation uniform location for the precomputed
  /// Normal matrix
  /// \param materialColorUniformLocation uniform location for the material
  /// diffuse color
  void setProgramUniformLocations(GLuint shaderProgramHandle,
                                  GLint mvpMtxUniformLocation,
                                  GLint normalMtxUniformLocation,
                                  GLint materialColorUniformLocation,
                                  GLint modelMtxUniformLocation);

  // moving
  // Gets the hero's position.
  glm::vec3 getPosition() const;

  // Sets the hero's position.
  void setPosition(glm::vec3 position);
  // Sets the hero's angle
  void setAngle(GLfloat angle);
  GLfloat getAngle();

    float getRadius() const { return _radius; }

  /// animation
  void _animateBro();

    void setHeading(const glm::vec3& heading);
  bool isAlive() const { return _alive; }
  bool isFalling() const { return _falling; }
  void setAlive(bool alive) { _alive = alive; }
  void setFalling(bool falling) { _falling = falling; }

    void update(float deltaTime, const glm::vec3& heroPosition, float turnSpeed);

private:
  // SHADER HANDLES AND LOCATIONS
  /// handle of the shader program to use when drawing the plane
  GLuint _shaderProgramHandle;
  /// stores the uniform locations needed for the plan information
  struct ShaderProgramUniformLocations {
    /// location of the precomputed ModelViewProjection matrix
    GLint mvpMtx;
    /// location of the precomputed Normal matrix
    GLint normalMtx;
    /// location of the material diffuse color
    GLint materialColor;

    GLint modelMtx;
  } _shaderProgramUniformLocations;

  // COMPONENT VALUES
  /// gonna be pi/2
  const GLfloat _halfPi;
  /// character's rotation
  GLfloat _rotationAngle;
  /// color the characters head
  const glm::vec3 _colorHead;
  /// amount to scale the head by
  const glm::vec3 _scaleHead;
  /// color the body
  const glm::vec3 _colorBody;
  /// amount to scale the body by
  const glm::vec3 _scaleBody;
    const glm::vec3 _colorCane;
  /// amount to scale the arm by
  const glm::vec3 _scaleArm;
  /// amount to scale the hand by
  const glm::vec3 _scaleHand;
  /// animation rotation value
  GLfloat _animOffset;
  GLfloat direction;
  glm::vec3 _position;

  glm::vec3 _headingVector;
  bool _alive;
  bool _falling;
  float _verticalVelocity;
  float _animPhase;
  float _moveSpeed;
  const float _radius;

  static constexpr GLfloat s_PI_OVER_2 = glm::half_pi<float>();

  // DRAWING TIME
  /// \desc draws just the character's body
  /// \param modelMtx existing model matrix to apply to character
  /// \param viewMtx camera view matrix to apply to character
  /// \param projMtx camera projection matrix to apply to character
  void _drawBrosHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                     const glm::mat4 &projMtx);
  /// \desc draws upper part of the body
  /// \param modelMtx existing model matrix to apply to character
  /// \param viewMtx camera view matrix to apply to character
  /// \param projMtx camera projection matrix to apply to character
  void _drawBrosUpperBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) const;
  void _drawBrosArm(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                    const glm::mat4 &projMtx) const;
  void _drawBrosHand(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                     const glm::mat4 &projMtx) const;
  void _drawBrosCane(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                     const glm::mat4 &projMtx) const;
  /// \desc draws lower part of the body
  /// \param modelMtx existing model matrix to apply to character
  /// \param viewMtx camera view matrix to apply to character
  /// \param projMtx camera projection matrix to apply to character
  void _drawBrosLowerBody(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) const;

  // EXPRESS DELIVERY
  /// \desc precomputes the matrix uniforms CPU-side and then sends them
  /// to the GPU to be used in the shader for each vertex.  It is more efficient
  /// to calculate these once and then use the resultant product in the shader.
  /// \param modelMtx model transformation matrix
  /// \param viewMtx camera view matrix
  /// \param projMtx camera projection matrix
  void _computeAndSendMatrixUniforms(const glm::mat4 &modelMtx,
                                     const glm::mat4 &viewMtx,
                                     const glm::mat4 &projMtx) const;
};

#endif // A3IMSOTIRED_WILFRED_H
