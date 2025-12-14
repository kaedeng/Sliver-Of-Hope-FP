#include "Tympanius.h"

#include <CSCI441/OpenGLEngine.hpp>
#include <iostream>

CSCI441::ModelLoader* Tympanius::_pHeadModel = nullptr;
CSCI441::ModelLoader* Tympanius::_pTailModel = nullptr;

Tympanius::Tympanius(
    CSCI441::ShaderProgram *shaderProgram,
    TympaniusShaderUniformLocations *shaderUniformLocations,
    TympaniusShaderAttributeLocations *shaderAttributeLocations)
    : _shaderProgram(shaderProgram),
      _shaderUniformLocations(shaderUniformLocations),
      _shaderAttributeLocations(shaderAttributeLocations) {
  _position = glm::vec3(0.0f, INIT_FLOATING_HEIGHT, 0.0f);
  _floatingHeight = INIT_FLOATING_HEIGHT;
  _heading = glm::vec3(1.0f, 0.0f, 0.0f);
  _tailHeading = glm::vec3(0.0f);

  if(_pHeadModel == nullptr){
    _pHeadModel = new CSCI441::ModelLoader();
    if (_pHeadModel->loadModelFile(
            "assets/models/heroes/Tympanius/Tympole_Head.obj")) {
      // TODO #18 - set attribute
      _pHeadModel->setAttributeLocations(_shaderAttributeLocations->vPos,
                                        _shaderAttributeLocations->vNormal,
                                        _shaderAttributeLocations->texCoord);
    } else {
      fprintf(stderr, "[ERROR]: Could not open OBJ Model\n");
      delete _pHeadModel;
      _pHeadModel = nullptr;
    }
  }

  if(_pTailModel == nullptr){
    _pTailModel = new CSCI441::ModelLoader();
    if (_pTailModel->loadModelFile(
            "assets/models/heroes/Tympanius/Tympole_Tail.obj")) {
      // TODO #18 - set attribute
      _pTailModel->setAttributeLocations(_shaderAttributeLocations->vPos,
                                        _shaderAttributeLocations->vNormal,
                                        _shaderAttributeLocations->texCoord);
    } else {
      fprintf(stderr, "[ERROR]: Could not open OBJ Model\n");
      delete _pTailModel;
      _pTailModel = nullptr;
    }
  }
}

Tympanius::~Tympanius() {}

// Renders the tympanius model by applying transformations and drawing each part
void Tympanius::draw(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx) {
  _shaderProgram->useProgram();
  // Apply tympanius's base transformations
  // modelMtx becomes the root of our hierarchy
  modelMtx = glm::translate(modelMtx, _position);
  modelMtx = glm::rotate(modelMtx, _heading[0], CSCI441::Y_AXIS);
  modelMtx = glm::scale(modelMtx, glm::vec3(_size));

  // Draw the head and tail
  _drawHead(modelMtx, viewMtx, projMtx);
  _drawTail(modelMtx, viewMtx, projMtx);
}

void Tympanius::update(GLfloat deltaTime, const glm::vec3& heroPosition, float turnSpeed) {
    _lastUpdatedTime += deltaTime;
    // move along heading
    _position += _heading * _moveSpeed * deltaTime;

    // calculate vector from enemy to hero
    glm::vec3 toHero = heroPosition - _position;
    toHero.y = 0.0f; // Only turn in horizontal plane

    // Tail waving animation
    _tailHeading[1] = -sin((2.0f + _lastUpdatedTime * 60) / 5.0f) * M_PI / 8.0f;

    // Tail rotation smoothly returns to center
    float returnSpeed = 0.05f;
    _tailHeading[0] = glm::mix(_tailHeading[0], 0.0f, returnSpeed / (deltaTime * 60));

    _position += _velocity;
    _floatingHeight = _position.y;

    // Floating animation - oscillate around floating height
    _position.y = _floatingHeight + 0.025f * sin(_lastUpdatedTime * 60 / 5.0f);

    if (glm::length(toHero) > 0.01f) {
        glm::vec3 desiredHeading = glm::normalize(toHero);

        // angle between current heading and new heading
        float currentAngle = _heading[0];
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
        _heading[0] = newAngle;
    }
}

void Tympanius::bounceOff(const glm::vec3& otherPosition) {
    // bounce direction after collision 
    glm::vec3 bounceDir = _position - otherPosition;
    bounceDir.y = 0.0f;

    if (glm::length(bounceDir) > 0.01f) {
        glm::vec3 headingVector = glm::normalize(bounceDir);
        _heading[0] = atan2(headingVector.x, headingVector.z);
    }
}

// Draws tympanius's head and eyes
void Tympanius::_drawHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) const {
  glm::mat4 headModelMtx = modelMtx;
  // _heading[0] rotation is already applied in the parent transform
  headModelMtx = glm::rotate(headModelMtx, _heading[1], CSCI441::X_AXIS);

  _setMatrices(headModelMtx, viewMtx, projMtx);
  _shaderProgram->setProgramUniform(_shaderUniformLocations->isTextured,
                                    GL_TRUE);
  _shaderProgram->setProgramUniform(_shaderUniformLocations->shininessAlpha,
                                    2.5f);

  _pHeadModel->draw(_shaderProgram->getShaderProgramHandle());
}

void Tympanius::_drawTail(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx) const {
  glm::mat4 tailModelMtx =
      glm::translate(modelMtx, glm::vec3(0.0f, 0.23f, -0.13f));
  tailModelMtx = glm::rotate(tailModelMtx, _tailHeading[0], CSCI441::Y_AXIS);
  tailModelMtx = glm::rotate(tailModelMtx, _tailHeading[1], CSCI441::X_AXIS);

  _setMatrices(tailModelMtx, viewMtx, projMtx);
  _shaderProgram->setProgramUniform(_shaderUniformLocations->isTextured,
                                    GL_TRUE);
  _shaderProgram->setProgramUniform(_shaderUniformLocations->shininessAlpha,
                                    2.5f);

  _pTailModel->draw(_shaderProgram->getShaderProgramHandle());
}

// Sends the MVP and normal matrices to the shader
void Tympanius::_setMatrices(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                             const glm::mat4 &projMtx) const {
  // Precompute the MVP matrix
  glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
  // Send MVP matrix to the shader
  glProgramUniformMatrix4fv(_shaderProgram->getShaderProgramHandle(),
                            _shaderUniformLocations->mvpMatrix, 1, GL_FALSE,
                            &mvpMtx[0][0]);
  
   glProgramUniformMatrix4fv(_shaderProgram->getShaderProgramHandle(), _shaderUniformLocations->modelMatrix, 1, GL_FALSE, &modelMtx[0][0]);

  // Compute and send the Normal matrix
  glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
  glProgramUniformMatrix3fv(_shaderProgram->getShaderProgramHandle(),
                            _shaderUniformLocations->normalMatrix, 1, GL_FALSE,
                            &normalMtx[0][0]);
}
