#include "Coin.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

GLuint Coin::_vao = 0;
GLuint Coin::_vbo = 0;
bool Coin::_buffersInitialized = false;

Coin::Coin(const glm::vec3& position)
    : _position(position),
      _collectionRadius(1.0f),
      _collected(false),
      _rotation(0.0f),
      _bobPhase(0.0f)
{
    if (!_buffersInitialized) {
        _initializeBuffers();
    }
}

void Coin::_initializeBuffers() {
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

void Coin::update(float deltaTime) {
    if (_collected) return;

    // spinning animation
    _rotation += deltaTime * 3.0f;
    if (_rotation > 2.0f * M_PI) {
        _rotation -= 2.0f * M_PI;
    }

    // bobbing animation
    _bobPhase += deltaTime * 2.0f;
    if (_bobPhase > 2.0f * M_PI) {
        _bobPhase -= 2.0f * M_PI;
    }
}

void Coin::draw(
    GLuint shaderProgramHandle,
    GLint mvpMatrixLoc,
    GLint textureLoc,
    const glm::mat4& viewMtx,
    const glm::mat4& projMtx,
    GLuint textureHandle
) const {
    if (_collected) return;

    glUseProgram(shaderProgramHandle);

    // create model matrix
    glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), _position);

    // add bobbing animation
    float bobAmount = sin(_bobPhase) * 0.3f;
    modelMtx = glm::translate(modelMtx, glm::vec3(0.0f, bobAmount, 0.0f));

    // get camera vecs from view matrix for billboarding
    glm::vec3 cameraRight = glm::vec3(viewMtx[0][0], viewMtx[1][0], viewMtx[2][0]);
    glm::vec3 cameraUp = glm::vec3(viewMtx[0][1], viewMtx[1][1], viewMtx[2][1]);

    // create billboard rotation matrix
    glm::mat4 billboardMtx = glm::mat4(1.0f);
    billboardMtx[0] = glm::vec4(cameraRight, 0.0f);
    billboardMtx[1] = glm::vec4(cameraUp, 0.0f);
    billboardMtx[2] = glm::vec4(glm::cross(cameraRight, cameraUp), 0.0f);

    modelMtx = modelMtx * billboardMtx;

    // spinning rotation around vertical axis
    modelMtx = glm::rotate(modelMtx, _rotation, glm::vec3(0.0f, 1.0f, 0.0f));

    // scale
    modelMtx = glm::scale(modelMtx, glm::vec3(1.0f, 1.0f, 1.0f));

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
