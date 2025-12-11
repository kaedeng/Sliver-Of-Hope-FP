#ifndef COIN_H
#define COIN_H

#include <glad/gl.h>
#include <glm/glm.hpp>

class Coin {
public:
    Coin(const glm::vec3& position);

    void update(float deltaTime);

    void draw(
        GLuint shaderProgramHandle,
        GLint mvpMatrixLoc,
        GLint textureLoc,
        const glm::mat4& viewMtx,
        const glm::mat4& projMtx,
        GLuint textureHandle
    ) const;

    glm::vec3 getPosition() const { return _position; }
    float getRadius() const { return _collectionRadius; }
    bool isCollected() const { return _collected; }

    void setCollected(bool collected) { _collected = collected; }

private:
    glm::vec3 _position;
    float _collectionRadius;
    bool _collected;
    float _rotation;
    float _bobPhase;

    static GLuint _vao;
    static GLuint _vbo;
    static bool _buffersInitialized;
    static void _initializeBuffers();
};

#endif
