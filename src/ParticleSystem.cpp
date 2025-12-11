#include "ParticleSystem.h"
#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// Static member initialization
GLuint ParticleSystem::_vao = 0;
GLuint ParticleSystem::_vbo = 0;
bool ParticleSystem::_buffersInitialized = false;

ParticleSystem::ParticleSystem() {
  if (!_buffersInitialized) {
    _initializeBuffers();
  }
}

ParticleSystem::~ParticleSystem() {
  // Note: We don't delete static buffers here as they may be used by other
  // instances
}

void ParticleSystem::_initializeBuffers() {
  // Create a billboarded quad (two triangles forming a square)
  struct Vertex {
    glm::vec3 position;
    glm::vec2 texCoord;
  };

  // Quad centered at origin, 1 unit size
  // Flipped vertically (Y texture coords inverted)
  Vertex vertices[] = {// Triangle 1
                       {glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
                       {glm::vec3(0.5f, -0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
                       {glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
                       // Triangle 2
                       {glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
                       {glm::vec3(0.5f, 0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
                       {glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec2(0.0f, 0.0f)}};

  glGenVertexArrays(1, &_vao);
  glBindVertexArray(_vao);

  glGenBuffers(1, &_vbo);
  glBindBuffer(GL_ARRAY_BUFFER, _vbo);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  // Position attribute (location 0)
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);

  // TexCoord attribute (location 1)
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                        (void *)offsetof(Vertex, texCoord));

  glBindVertexArray(0);

  _buffersInitialized = true;
}

void ParticleSystem::spawnBurst(const glm::vec3 &position, int numParticles) {
  for (int i = 0; i < numParticles; ++i) {
    Particle particle;
    particle.position = position;

    // Random velocity in all directions
    float angle = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * M_PI;
    float elevation =
        (static_cast<float>(rand()) / RAND_MAX - 0.5f) * M_PI * 0.5f;
    float speed = 3.0f + (static_cast<float>(rand()) / RAND_MAX) * 5.0f;

    particle.velocity =
        glm::vec3(cos(elevation) * cos(angle) * speed,
                  sin(elevation) * speed + 5.0f, // Add upward velocity
                  cos(elevation) * sin(angle) * speed);

    particle.maxLifetime =
        1.0f + (static_cast<float>(rand()) / RAND_MAX) * 1.0f;
    particle.lifetime = particle.maxLifetime;
    particle.size = 0.3f + (static_cast<float>(rand()) / RAND_MAX) * 0.3f;
    particle.rotation = (static_cast<float>(rand()) / RAND_MAX) * 2.0f * M_PI;
    particle.rotationSpeed =
        ((static_cast<float>(rand()) / RAND_MAX) - 0.5f) * 10.0f;
    particle.active = true;

    _particles.push_back(particle);
  }
}

void ParticleSystem::update(float deltaTime) {
  const float gravity = -20.0f;

  for (auto &particle : _particles) {
    if (!particle.active)
      continue;

    // Apply gravity
    particle.velocity.y += gravity * deltaTime;

    // Update position
    particle.position += particle.velocity * deltaTime;

    // Update rotation
    particle.rotation += particle.rotationSpeed * deltaTime;

    // Update lifetime
    particle.lifetime -= deltaTime;

    if (particle.lifetime <= 0.0f) {
      particle.active = false;
    }
  }

  // Remove inactive particles to prevent memory growth
  _particles.erase(std::remove_if(_particles.begin(), _particles.end(),
                                  [](const Particle &p) { return !p.active; }),
                   _particles.end());
}

void ParticleSystem::draw(GLuint shaderProgramHandle, GLint mvpMatrixLoc,
                          GLint textureLoc, const glm::mat4 &viewMtx,
                          const glm::mat4 &projMtx, GLuint textureHandle) {
  if (_particles.empty())
    return;

  glUseProgram(shaderProgramHandle);

  // Enable blending for transparency
  glEnable(GL_BLEND);
  glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

  // Bind texture
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, textureHandle);
  glUniform1i(textureLoc, 0);

  // Extract camera vectors for billboarding
  glm::vec3 cameraRight =
      glm::vec3(viewMtx[0][0], viewMtx[1][0], viewMtx[2][0]);
  glm::vec3 cameraUp = glm::vec3(viewMtx[0][1], viewMtx[1][1], viewMtx[2][1]);

  glBindVertexArray(_vao);

  for (const auto &particle : _particles) {
    if (!particle.active)
      continue;

    // Create model matrix for the particle
    glm::mat4 modelMtx = glm::translate(glm::mat4(1.0f), particle.position);

    // Build billboard rotation matrix
    glm::mat4 billboardMtx = glm::mat4(1.0f);
    billboardMtx[0] = glm::vec4(cameraRight, 0.0f);
    billboardMtx[1] = glm::vec4(cameraUp, 0.0f);
    billboardMtx[2] = glm::vec4(glm::cross(cameraRight, cameraUp), 0.0f);

    modelMtx = modelMtx * billboardMtx;

    // Add rotation
    modelMtx =
        glm::rotate(modelMtx, particle.rotation, glm::vec3(0.0f, 0.0f, 1.0f));

    // Scale based on size
    modelMtx = glm::scale(modelMtx, glm::vec3(particle.size));

    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;

    // Set MVP uniform
    glUniformMatrix4fv(mvpMatrixLoc, 1, GL_FALSE, glm::value_ptr(mvpMtx));

    // Draw particle
    glDrawArrays(GL_TRIANGLES, 0, 6);
  }

  glBindVertexArray(0);
  glDisable(GL_BLEND);
}
