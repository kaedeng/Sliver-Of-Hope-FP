#include "Enemy.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <cmath>

Enemy::Enemy() = default;

Enemy::~Enemy() = default;

glm::vec3 Enemy::getPosition() const { return glm::vec3(-1, -1, -1); }
void Enemy::setPosition(glm::vec3 position) {};
float Enemy::getRadius() const { return -1.0f;}
void Enemy::bounceOff(const glm::vec3 &otherPosition) {};