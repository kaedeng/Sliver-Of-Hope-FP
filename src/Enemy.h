#ifndef ENEMY_H
#define ENEMY_H

#include <glad/gl.h>
#include <glm/glm.hpp>

// Pure virtual class for a unified interface on enemies
class Enemy {
public:
    Enemy();
    virtual ~Enemy();
    virtual glm::vec3 getPosition() const;
    virtual void setPosition(glm::vec3 position);
    virtual float getRadius() const;
    virtual void bounceOff(const glm::vec3 &otherPosition);
};

#endif 
