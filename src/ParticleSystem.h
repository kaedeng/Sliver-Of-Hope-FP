#ifndef PARTICLE_SYSTEM_H
#define PARTICLE_SYSTEM_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <vector>

class ParticleSystem {
public:
    ParticleSystem();
    ~ParticleSystem();

    // Spawn a burst of particles at a position
    void spawnBurst(const glm::vec3& position, int numParticles = 20);

    // Update all particles
    void update(float deltaTime);

    // Draw all particles
    void draw(GLuint shaderProgramHandle,
              GLint mvpMatrixLoc,
              GLint textureLoc,
              const glm::mat4& viewMtx,
              const glm::mat4& projMtx,
              GLuint textureHandle);

private:
    struct Particle {
        glm::vec3 position;
        glm::vec3 velocity;
        float lifetime;
        float maxLifetime;
        float size;
        float rotation;
        float rotationSpeed;
        bool active;
    };

    std::vector<Particle> _particles;

    // Static VAO/VBO for particle rendering
    static GLuint _vao;
    static GLuint _vbo;
    static bool _buffersInitialized;
    static void _initializeBuffers();
};

#endif // PARTICLE_SYSTEM_H
