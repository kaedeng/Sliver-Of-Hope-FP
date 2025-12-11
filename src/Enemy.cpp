#include "Enemy.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

GLuint Enemy::_vao = 0;
GLuint Enemy::_vbo = 0;
bool Enemy::_buffersInitialized = false;

Enemy::Enemy(const glm::vec3& position, float heading)
    : _position(position),
      _radius(0.5f),
      _moveSpeed(5.0f),
      _alive(true),
      _falling(false),
      _verticalVelocity(0.0f),
      _animPhase(0.0f)
{
    _headingVector = glm::normalize(glm::vec3(sin(heading), 0.0f, cos(heading)));

    if (!_buffersInitialized) {
        _initializeBuffers();
    }
}

void Enemy::_initializeBuffers() {
    struct Vertex {
        glm::vec3 position;
        glm::vec2 texCoord;
    };

    Vertex vertices[] = {
        // triangle 1
        { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f) },
        { glm::vec3( 0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f) },
        { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 0.0f) },
        // triangle 2
        { glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f) },
        { glm::vec3( 0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 0.0f) },
        { glm::vec3(-0.5f,  0.5f, 0.0f), glm::vec2(0.0f, 0.0f) }
    };

    glGenVertexArrays(1, &_vao);
    glBindVertexArray(_vao);

    glGenBuffers(1, &_vbo);
    glBindBuffer(GL_ARRAY_BUFFER, _vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    // position (location 0)
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);

    // texCoord (location 1)
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));

    glBindVertexArray(0);

    _buffersInitialized = true;
}

void Enemy::setHeading(const glm::vec3& heading) {
    _headingVector = glm::normalize(heading);
}

void Enemy::update(float deltaTime, const glm::vec3& heroPosition, float turnSpeed) {
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
        _headingVector = glm::normalize(glm::vec3(sin(newAngle), 0.0f, cos(newAngle)));
    }

    // Update 
    _animPhase += deltaTime * 5.0f;
    if (_animPhase > 2.0f * M_PI) {
        _animPhase -= 2.0f * M_PI;
    }
}

void Enemy::bounceOff(const glm::vec3& otherPosition) {
    // bounce direction after collision 
    glm::vec3 bounceDir = _position - otherPosition;
    bounceDir.y = 0.0f;

    if (glm::length(bounceDir) > 0.01f) {
        _headingVector = glm::normalize(bounceDir);
    }
}

void Enemy::draw(
    GLuint shaderProgramHandle,
    GLint mvpMatrixLoc,
    GLint textureLoc,
    const glm::mat4& viewMtx,
    const glm::mat4& projMtx,
    GLuint textureHandle
) const {
    if (!_alive) return;

    glUseProgram(shaderProgramHandle);

    // create model matrix 
    glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), _position);

    // get camera vecs from view matrix for billboarding
    glm::vec3 cameraRight = glm::vec3(viewMtx[0][0], viewMtx[1][0], viewMtx[2][0]);
    glm::vec3 cameraUp = glm::vec3(viewMtx[0][1], viewMtx[1][1], viewMtx[2][1]);

    // create billboard rotation matrix
    glm::mat4 billboardMtx = glm::mat4(1.0f);
    billboardMtx[0] = glm::vec4(cameraRight, 0.0f);
    billboardMtx[1] = glm::vec4(cameraUp, 0.0f);
    billboardMtx[2] = glm::vec4(glm::cross(cameraRight, cameraUp), 0.0f);

    modelMtx = modelMtx * billboardMtx;

    // add bobbing animation
    float bobAmount = sin(_animPhase) * 0.1f;
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, bobAmount, 0.0f));

    // scale
    modelMtx = glm::scale(modelMtx, glm::vec3(1.5f, 1.5f, 1.5f));

    // if falling, add rotation so the goomba looks like that one kirby falling animation lol
    if (_falling) {
        float rotationAngle = _verticalVelocity * 0.1f;
        modelMtx = glm::rotate(modelMtx, rotationAngle, glm::vec3(1.0f, 0.0f, 0.0f));
    }

    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;

    // uniforms
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvpMtx));

    // bind texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureHandle);
    glUniform1i(textureLoc, 0);

    // enable blending for transparent pixels
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // draw
    glBindVertexArray(_vao);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    glBindVertexArray(0);

    glDisable(GL_BLEND); // turn this off after drawing
}
