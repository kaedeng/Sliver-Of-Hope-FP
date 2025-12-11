#ifndef ENEMY_H
#define ENEMY_H

#include <glad/gl.h>
#include <glm/glm.hpp>

class Enemy {
public:
    Enemy(const glm::vec3& position, float heading);

    void update(float deltaTime, const glm::vec3& heroPosition, float turnSpeed);

    void draw(
        GLuint shaderProgramHandle,
        GLint mvpMatrixLoc,
        GLint textureLoc,
        const glm::mat4& viewMtx,
        const glm::mat4& projMtx,
        GLuint textureHandle
    ) const;

    glm::vec3 getPosition() const { return _position; }
    glm::vec3 getHeading() const { return _headingVector; }
    float getRadius() const { return _radius; }
    bool isAlive() const { return _alive; }
    bool isFalling() const { return _falling; }

    void setPosition(const glm::vec3& position) { _position = position; }
    void setHeading(const glm::vec3& heading);
    void setAlive(bool alive) { _alive = alive; }
    void setFalling(bool falling) { _falling = falling; }

    void bounceOff(const glm::vec3& otherPosition);

    float getAnimationPhase() const { return _animPhase; }

private:
    glm::vec3 _position;
    glm::vec3 _headingVector;
    float _radius;
    float _moveSpeed;
    bool _alive;
    bool _falling;
    float _verticalVelocity;
    float _animPhase;

    static GLuint _vao;
    static GLuint _vbo;
    static bool _buffersInitialized;
    static void _initializeBuffers();
};

#endif 
