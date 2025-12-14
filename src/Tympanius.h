#ifndef TYMPANIUS_H
#define TYMPANIUS_H

#include <glad/gl.h>
#include <glm/glm.hpp>

#include <CSCI441/objects.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "include/CSCI441/ModelLoader.hpp"
#include <CSCI441/ShaderProgram.hpp>
#include "TympaniusShaderUniformLocations.hpp"
#include "TympaniusShaderAttributeLocations.hpp"

class Tympanius
{
public:
    Tympanius(CSCI441::ShaderProgram *shaderProgram, TympaniusShaderUniformLocations* shaderUniformLocations, TympaniusShaderAttributeLocations* shaderAttributeLocations);
    ~Tympanius();

    // Updates tympanius' state.
    void update(GLfloat deltaTime, const glm::vec3& heroPosition, float turnSpeed);

    // Bounce off other objects
    void bounceOff(const glm::vec3& otherPosition);

    // Draws tympanius
    void draw(glm::mat4 modelMtx, glm::mat4 viewMtx, glm::mat4 projMtx);

    // Returns tympanius's position
    glm::vec3 getPosition() const { return _position; }

    // Sets tympanius's position
    void setPosition(glm::vec3 position) { _position = glm::vec3(position.x, position.y+_floatingHeight, position.z); }

    // Returns tympanius's heading
    glm::vec3 getHeading() const { return _heading; }

    // Sets tympanius's angle
    void setHeading(glm::vec3 heading) { _heading = heading; }

    // Returns tympanius's tail heading
    glm::vec3 getTailHeading() const { return _tailHeading; }

    // Sets tympanius's angle
    void setTailHeading(glm::vec3 heading) { _tailHeading = heading; }

    // Get y coordinate that tympanius hovers around
    GLfloat getfloatingHeight() const { return _floatingHeight; }

    void setFloatingHeight(GLfloat floatingHeight) { _floatingHeight = floatingHeight; }

    GLfloat getMoveSpeed() const { return _moveSpeed; }

    glm::vec3 getVelocity() const { return _velocity; }

    void setVelocity(glm::vec3 velocity) { _velocity = velocity; }
    
    GLfloat getRadius() const { return 1.75f; }
    
    bool isFalling() const { return _falling; }

    void setFalling(bool falling) { _falling = falling; }
   
    bool isAlive() const { return _alive; }

    void setAlive(bool alive) { _alive = alive; }
    
    
    const GLfloat ORIGINAL_SIZE = 10.0f;
    const short MAX_FALL_COUNT = 5;
    const GLfloat INIT_FLOATING_HEIGHT = 0.75f;
    const GLfloat DAMPING = 0.93f;
    const GLfloat GRAVITY = 0.02f;
    const GLfloat MAX_INTANGIBLE_TIME = 3.0f;

private:
    glm::vec3 _position;
    glm::vec3 _heading;
    GLfloat _size = ORIGINAL_SIZE;
    glm::vec3 _tailHeading;
    GLfloat _floatingHeight;
    GLfloat _moveSpeed = 5.0f;
    glm::vec3 _velocity;
    GLfloat _lastUpdatedTime = 0.0f;
    bool _falling = false;
    bool _alive = true;

    // \desc holds tympanius model objects
    static CSCI441::ModelLoader *_pHeadModel;
    static CSCI441::ModelLoader *_pTailModel;

    CSCI441::ShaderProgram *_shaderProgram;
    TympaniusShaderUniformLocations *_shaderUniformLocations;
    TympaniusShaderAttributeLocations *_shaderAttributeLocations;

    // Draws the head and eyes relative to the hero's root.
    void _drawHead(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                   const glm::mat4 &projMtx) const;

    // Draws a hand.
    void _drawTail(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                   const glm::mat4 &projMtx) const;

    // Helper function to send matrix uniforms to the shader.
    void _setMatrices(glm::mat4 modelMtx, const glm::mat4 &viewMtx,
                      const glm::mat4 &projMtx) const;
};

#endif // HERO_H
