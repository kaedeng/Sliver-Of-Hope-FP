#ifndef CHARACTER_H
#define CHARACTER_H

#include <glad/gl.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <string>
#include <vector>
#include <map>

// forward declaration of required gltf stuff
namespace tinygltf {
    class Model;
    class Node;
    class Animation;
}

class Character {
public:
    Character(
        GLuint shaderProgramHandle,
        GLint mvpMtxUniformLocation,
        GLint normalMtxUniformLocation,
        GLint modelMtxUniformLocation,
        GLint materialDiffuseLocation,
        GLint materialSpecularLocation,
        GLint materialShininessLocation
    );
    
    ~Character();
    
    // load character from gltf file
    bool loadFromFile(const std::string& filepath);
    
    // draw the character
    void draw(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx);
    
    // animation control functions
    void playAnimation(const std::string& animationName);
    void update(float deltaTime);
    void update(float deltaTime, const glm::vec3& targetPosition, float turnSpeed);
    
    // movement functions
    void moveForward(float amount);
    void moveBackward(float amount);
    void turnLeft(float angle);
    void turnRight(float angle);
    
    glm::vec3 getPosition() const { return _position; }
    void setPosition(const glm::vec3& position);
    float getHeading() const { return _heading; }

    // Update shader references after shader reload
    void updateShaderReferences(
        GLuint shaderProgramHandle,
        GLint mvpMtxUniformLocation,
        GLint normalMtxUniformLocation,
        GLint modelMtxUniformLocation,
        GLint materialDiffuseLocation,
        GLint materialSpecularLocation,
        GLint materialShininessLocation
    );
    
private:
    // shader info
    GLuint _shaderProgramHandle;
    struct ShaderLocations {
        GLint mvpMtx;
        GLint normalMtx;
        GLint modelMtx;
        GLint materialDiffuse;
        GLint materialSpecular;
        GLint materialShininess;
        GLint jointMatrices; // for skeleton animation
    } _shaderLocations;
    
    // character transform
    glm::vec3 _position;
    float _heading; // rotation around Y axis
    glm::vec3 _headingVector; // normalized direction vector for pathfinding
    float _moveSpeed; // movement speed for enemy AI
    
    // animation states
    struct AnimationState {
        int currentAnimation;
        float currentTime;
        bool isPlaying;
    } _animState;
    
    // gltf model data
    tinygltf::Model* _model;
    
    // mesh data
    struct Primitive {
        GLuint vao;
        GLuint vbo_positions;
        GLuint vbo_normals;
        GLuint vbo_texcoords;
        GLuint vbo_joints; // bone indices affecting each vertex
        GLuint vbo_weights; // weight of each bone influence
        GLuint ibo;
        int indexCount;
        GLenum indexType;       // GL_UNSIGNED_BYTE, GL_UNSIGNED_SHORT, or GL_UNSIGNED_INT
        size_t indexByteOffset; // byte offset in IBO
        int materialIndex;
        bool hasTexCoords;
    };
    std::vector<Primitive> _primitives;
    
    // skeleton data
    struct Joint {
        std::string name;
        int parentIndex;
        glm::mat4 inverseBindMatrix; // Brings vertex from mesh space to bone space
        glm::mat4 localTransform; // Current transform relative to parent
        glm::mat4 globalTransform; // Current transform in model space
    };
    std::vector<Joint> _joints;
    std::vector<glm::mat4> _jointMatrices; // final matrices sent to shader
    
    // animation data
    struct AnimationClip {
        std::string name; // only elsterWalking or elsterIdle
        float duration;
        
        struct Channel { // theres 700+ duplicate channels and I'm not sure what the problem is and how to fix it. It's not causing a problem though.
            int jointIndex;
            enum Type { TRANSLATION, ROTATION, SCALE } type;
            std::vector<float> times;
            std::vector<glm::vec3> translations;
            std::vector<glm::quat> rotations;
            std::vector<glm::vec3> scales;
        };
        std::vector<Channel> channels;
    };
    std::vector<AnimationClip> _animations;
    
    // material data
    struct Material {
        glm::vec3 diffuse;
        glm::vec3 specular;
        float shininess;
        GLuint textureID;
        bool hasTexture;
    };
    std::vector<Material> _materials;

    // texture loader
    GLuint _loadTextureFromGLTF(int textureIndex);
    
    // loading functions
    void _loadMeshes();
    void _loadSkeleton();
    void _loadAnimations();
    void _loadMaterials();
    void _setupMeshBuffers(int meshIndex);
    
    // animation functions
    void _updateAnimation(float deltaTime);
    void _updateJointTransforms();
    glm::vec3 _interpolateVec3(
        const std::vector<float>& times, 
        const std::vector<glm::vec3>& values, 
        float time
    );
    glm::quat _interpolateQuat(
        const std::vector<float>& times,
        const std::vector<glm::quat>& values,
        float time
    );
    
    // rendering functions
    void _renderNode(
        int nodeIndex, const glm::mat4& parentTransform,
        const glm::mat4& viewMtx, const glm::mat4& projMtx
    );
    void _computeAndSendMatrixUniforms(
        const glm::mat4& modelMtx,    
        const glm::mat4& viewMtx,       
        const glm::mat4& projMtx
    );
};

#endif // CHARACTER_H