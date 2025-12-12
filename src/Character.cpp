#include "Character.h"

#define TINYGLTF_IMPLEMENTATION
#include <tiny_gltf.h>

#include <iostream>
#include <unordered_map>

Character::Character(
    GLuint shaderProgramHandle,
    GLint mvpMtxUniformLocation,
    GLint normalMtxUniformLocation,
    GLint modelMtxUniformLocation,
    GLint materialDiffuseLocation,
    GLint materialSpecularLocation,
    GLint materialShininessLocation
) : _shaderProgramHandle(shaderProgramHandle),
    _position(0.0f, 0.0f, 0.0f),
    _heading(0.0f),
    _headingVector(0.0f, 0.0f, 1.0f),
    _moveSpeed(5.0f),
    _model(nullptr)
{
    _shaderLocations.mvpMtx = mvpMtxUniformLocation;
    _shaderLocations.normalMtx = normalMtxUniformLocation;
    _shaderLocations.modelMtx = modelMtxUniformLocation;
    _shaderLocations.materialDiffuse = materialDiffuseLocation;
    _shaderLocations.materialSpecular = materialSpecularLocation;
    _shaderLocations.materialShininess = materialShininessLocation;

    _shaderLocations.jointMatrices = glGetUniformLocation(shaderProgramHandle, "jointMatrices");

    _animState.currentAnimation = -1;
    _animState.currentTime = 0.0f;
    _animState.isPlaying = false;
}

Character::~Character() {
    for (auto& prim : _primitives) {
        glDeleteVertexArrays(1, &prim.vao);
        glDeleteBuffers(1, &prim.vbo_positions);
        glDeleteBuffers(1, &prim.vbo_normals);
        if (prim.vbo_texcoords) glDeleteBuffers(1, &prim.vbo_texcoords);
        if (prim.vbo_joints) glDeleteBuffers(1, &prim.vbo_joints);
        if (prim.vbo_weights) glDeleteBuffers(1, &prim.vbo_weights);
        if (prim.ibo) glDeleteBuffers(1, &prim.ibo);
    }

    // clearnup textures
    for (auto& mat : _materials) {
        if (mat.textureID != 0) {
            glDeleteTextures(1, &mat.textureID);
        }
    }

    delete _model;
}

bool Character::loadFromFile(const std::string& filepath) {
    tinygltf::TinyGLTF loader;
    std::string err, warn;
    
    _model = new tinygltf::Model();
    
    bool ret = false;
    ret = loader.LoadBinaryFromFile(_model, &err, &warn, filepath);
    
    if (!warn.empty()) {
        std::cout << "GLTF Warning: " << warn << std::endl;
    }
    
    if (!err.empty()) {
        std::cerr << "GLTF Error: " << err << std::endl;
    }
    
    if (!ret) {
        std::cerr << "Failed to load glTF" << std::endl;
        return false;
    }
    
    // Load components
    _loadMeshes();
    _loadSkeleton();
    _loadAnimations();
    _loadMaterials();
    
    std::cout << "Successfully loaded character from " << filepath << std::endl;
    std::cout << "  Primitives: " << _primitives.size() << std::endl;
    std::cout << "  Joints: " << _joints.size() << std::endl;
    std::cout << "  Animations: " << _animations.size() << std::endl;
    std::cout << "  Materials: " << _materials.size() << std::endl;
    
    // Start with idle animation
    if (!_animations.empty()) {
        playAnimation("elsterIdle");
    }
    
    if (!_joints.empty() && _jointMatrices.empty()) _updateJointTransforms();

    return true;
}

void Character::_loadMeshes() {
    for (size_t i = 0; i < _model->meshes.size(); ++i) {
        _setupMeshBuffers(i);
    }
}

void Character::_setupMeshBuffers(int meshIndex) {
    const tinygltf::Mesh& mesh = _model->meshes[meshIndex];

    for (const auto& primitive : mesh.primitives) {
        Primitive prim = {};
        prim.materialIndex = primitive.material;
        prim.hasTexCoords = false;
        prim.vbo_texcoords = 0;
        prim.vbo_joints = 0;
        prim.vbo_weights = 0;
        prim.ibo = 0;

        // create VAO
        glGenVertexArrays(1, &prim.vao);
        glBindVertexArray(prim.vao);

        // load positions
        if (primitive.attributes.find("POSITION") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.at("POSITION")];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            size_t byteStride = accessor.ByteStride(bufferView);

            glGenBuffers(1, &prim.vbo_positions);
            glBindBuffer(GL_ARRAY_BUFFER, prim.vbo_positions);
            glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, byteStride, (void*)(size_t)accessor.byteOffset);
            glEnableVertexAttribArray(0);
        }

        // load normals
        if (primitive.attributes.find("NORMAL") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.at("NORMAL")];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            size_t byteStride = accessor.ByteStride(bufferView);

            glGenBuffers(1, &prim.vbo_normals);
            glBindBuffer(GL_ARRAY_BUFFER, prim.vbo_normals);
            glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, byteStride, (void*)(size_t)accessor.byteOffset);
            glEnableVertexAttribArray(1);
        }

        // load texture coordinates
        if (primitive.attributes.find("TEXCOORD_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.at("TEXCOORD_0")];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            size_t byteStride = accessor.ByteStride(bufferView);

            glGenBuffers(1, &prim.vbo_texcoords);
            glBindBuffer(GL_ARRAY_BUFFER, prim.vbo_texcoords);
            glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
            glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, byteStride, (void*)(size_t)accessor.byteOffset);
            glEnableVertexAttribArray(4);
            prim.hasTexCoords = true;
        }

        // load joint indices
        if (primitive.attributes.find("JOINTS_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.at("JOINTS_0")];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            size_t byteStride = accessor.ByteStride(bufferView);

            glGenBuffers(1, &prim.vbo_joints);
            glBindBuffer(GL_ARRAY_BUFFER, prim.vbo_joints);
            glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);

            // tinygltf stores joints in some weird random formats so check before using them
            // i spent too much time figuring this one out
            GLenum glType = GL_UNSIGNED_SHORT; // fallback
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    glType = GL_UNSIGNED_BYTE;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    glType = GL_UNSIGNED_SHORT;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    glType = GL_UNSIGNED_INT;
                    break;
                default:
                    std::cerr << "Unknown JOINTS_0 componentType: " << accessor.componentType << std::endl;
            }

            glVertexAttribIPointer(2, 4, glType, byteStride, (void*)(size_t)accessor.byteOffset);
            glEnableVertexAttribArray(2);
        }

        // load joint weights
        if (primitive.attributes.find("WEIGHTS_0") != primitive.attributes.end()) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.attributes.at("WEIGHTS_0")];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            size_t byteStride = accessor.ByteStride(bufferView);

            glGenBuffers(1, &prim.vbo_weights);
            glBindBuffer(GL_ARRAY_BUFFER, prim.vbo_weights);
            glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
            glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, byteStride, (void*)(size_t)accessor.byteOffset);
            glEnableVertexAttribArray(3);
        }

        // load indices
        if (primitive.indices >= 0) {
            const tinygltf::Accessor& accessor = _model->accessors[primitive.indices];
            const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
            const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];

            prim.indexCount = accessor.count;
            prim.indexByteOffset = accessor.byteOffset;

            // same shenanigans as above
            switch (accessor.componentType) {
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_BYTE:
                    prim.indexType = GL_UNSIGNED_BYTE;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT:
                    prim.indexType = GL_UNSIGNED_SHORT;
                    break;
                case TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT:
                    prim.indexType = GL_UNSIGNED_INT;
                    break;
                default:
                    prim.indexType = GL_UNSIGNED_SHORT;
                    std::cerr << "Unknown index type, defaulting to GL_UNSIGNED_SHORT" << std::endl;
            }

            glGenBuffers(1, &prim.ibo);
            glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, prim.ibo);
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, bufferView.byteLength, &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);
        }

        glBindVertexArray(0);
        _primitives.push_back(prim);
    }
}

// need to handle all parental relationships between joints and also inital locations
void Character::_loadSkeleton() {
    // find the skin
    if (_model->skins.empty()) {
        std::cout << "No skeleton found in model" << std::endl;
        return;
    }
    
    const tinygltf::Skin& skin = _model->skins[0];
    
    const tinygltf::Accessor& accessor = _model->accessors[skin.inverseBindMatrices];
    const tinygltf::BufferView& bufferView = _model->bufferViews[accessor.bufferView];
    const tinygltf::Buffer& buffer = _model->buffers[bufferView.buffer];
    
    const float* matrices = reinterpret_cast<const float*>(
        &buffer.data[bufferView.byteOffset + accessor.byteOffset]);
    
    // initialize joints
    _joints.resize(skin.joints.size());
    _jointMatrices.resize(skin.joints.size());
    
    for (size_t i = 0; i < skin.joints.size(); ++i) {
        int nodeIndex = skin.joints[i];
        const tinygltf::Node& node = _model->nodes[nodeIndex];
        
        _joints[i].name = node.name;
        _joints[i].inverseBindMatrix = glm::make_mat4(&matrices[i * 16]);
        
        // find parent
        _joints[i].parentIndex = -1;
        for (size_t j = 0; j < _model->nodes.size(); ++j) {
            const auto& parentNode = _model->nodes[j];
            auto it = std::find(parentNode.children.begin(), parentNode.children.end(), nodeIndex);
            if (it != parentNode.children.end()) {
                for (size_t k = 0; k < skin.joints.size(); ++k) {
                    if (skin.joints[k] == (int)j) {
                        _joints[i].parentIndex = k;
                        break;
                    }
                }
                break;
            }
        }
        
        // get initial transform
        glm::mat4 localTransform(1.0f);
        if (!node.matrix.empty()) {
            localTransform = glm::make_mat4(node.matrix.data());
        } else {
            if (!node.translation.empty()) {
                localTransform = glm::translate(localTransform, 
                    glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
            }
            if (!node.rotation.empty()) {
                glm::quat rot(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
                localTransform *= glm::mat4_cast(rot);
            }
            if (!node.scale.empty()) {
                localTransform = glm::scale(localTransform,
                    glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
            }
        }
        _joints[i].localTransform = localTransform;
    }
}

// setup the animations from the gltf model
void Character::_loadAnimations() {
    std::cout << "Loading " << _model->animations.size() << " animations:" << std::endl;

    for (const auto& anim : _model->animations) {
        AnimationClip clip;
        clip.name = anim.name;
        clip.duration = 0.0f;

        std::cout << "  - Animation: \"" << anim.name << "\"" << std::endl;

        // FIX: Use a map to track channels and filter duplicates
        // Key: (jointIndex, channelType), Value: channel index
        std::unordered_map<std::string, size_t> channelMap;
        std::vector<AnimationClip::Channel> tempChannels;

        for (const auto& channel : anim.channels) {
            const tinygltf::AnimationSampler& sampler = anim.samplers[channel.sampler];
            
            // find which joint this channel affects, again, theres like 700+ duplicate channels, but it doesn't seem to be bad
            int jointIndex = -1;
            int targetNode = channel.target_node;
            const tinygltf::Skin& skin = _model->skins[0];
            for (size_t i = 0; i < skin.joints.size(); ++i) {
                if (skin.joints[i] == targetNode) {
                    jointIndex = i;
                    break;
                }
            }
            
            if (jointIndex < 0) continue;
            
            AnimationClip::Channel animChannel;
            animChannel.jointIndex = jointIndex;
            
            // load times
            const tinygltf::Accessor& timeAccessor = _model->accessors[sampler.input];
            const tinygltf::BufferView& timeBufferView = _model->bufferViews[timeAccessor.bufferView];
            const tinygltf::Buffer& timeBuffer = _model->buffers[timeBufferView.buffer];
            const float* times = reinterpret_cast<const float*>(
                &timeBuffer.data[timeBufferView.byteOffset + timeAccessor.byteOffset]);
            
            animChannel.times.assign(times, times + timeAccessor.count);
            
            // update duration
            if (!animChannel.times.empty()) {
                clip.duration = std::max(clip.duration, animChannel.times.back());
            }
            
            // load values
            const tinygltf::Accessor& valueAccessor = _model->accessors[sampler.output];
            const tinygltf::BufferView& valueBufferView = _model->bufferViews[valueAccessor.bufferView];
            const tinygltf::Buffer& valueBuffer = _model->buffers[valueBufferView.buffer];
            const float* values = reinterpret_cast<const float*>(
                &valueBuffer.data[valueBufferView.byteOffset + valueAccessor.byteOffset]);
            
            // set channel type
            std::string channelKey;
            if (channel.target_path == "translation") {
                animChannel.type = AnimationClip::Channel::TRANSLATION;
                channelKey = std::to_string(jointIndex) + "_T";
                for (size_t i = 0; i < valueAccessor.count; ++i) {
                    animChannel.translations.push_back(glm::vec3(values[i*3], values[i*3+1], values[i*3+2]));
                }
            } else if (channel.target_path == "rotation") {
                animChannel.type = AnimationClip::Channel::ROTATION;
                channelKey = std::to_string(jointIndex) + "_R";
                for (size_t i = 0; i < valueAccessor.count; ++i) {
                    animChannel.rotations.push_back(glm::quat(values[i*4+3], values[i*4], values[i*4+1], values[i*4+2]));
                }
            } else if (channel.target_path == "scale") {
                animChannel.type = AnimationClip::Channel::SCALE;
                channelKey = std::to_string(jointIndex) + "_S";
                for (size_t i = 0; i < valueAccessor.count; ++i) {
                    animChannel.scales.push_back(glm::vec3(values[i*3], values[i*3+1], values[i*3+2]));
                }
            } else {
                continue; // unknown channel type, shouldn't ever reach here
            }
            
            auto it = channelMap.find(channelKey);
            if (it != channelMap.end()) {
                tempChannels[it->second] = animChannel;
            } else {
                channelMap[channelKey] = tempChannels.size();
                tempChannels.push_back(animChannel);
            }
        }
        
        clip.channels = tempChannels;

        _animations.push_back(clip);
    }
}


// find and load the texure from the gltf model
GLuint Character::_loadTextureFromGLTF(int textureIndex) {
    if (textureIndex < 0 || textureIndex >= (int)_model->textures.size()) {
        return 0;
    }

    const tinygltf::Texture& tex = _model->textures[textureIndex];
    if (tex.source < 0 || tex.source >= (int)_model->images.size()) {
        return 0;
    }

    const tinygltf::Image& image = _model->images[tex.source];

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);

    GLenum format = GL_RGBA;
    if (image.component == 1) {
        format = GL_RED;
    } else if (image.component == 2) {
        format = GL_RG;
    } else if (image.component == 3) {
        format = GL_RGB;
    } else if (image.component == 4) {
        format = GL_RGBA;
    }

    GLenum type = GL_UNSIGNED_BYTE;
    if (image.bits == 8) {
        type = GL_UNSIGNED_BYTE;
    } else if (image.bits == 16) {
        type = GL_UNSIGNED_SHORT;
    }

    glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0,
                 format, type, &image.image[0]);

    if (tex.sampler >= 0 && tex.sampler < (int)_model->samplers.size()) {
        const tinygltf::Sampler& sampler = _model->samplers[tex.sampler];

        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                       sampler.minFilter != -1 ? sampler.minFilter : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                       sampler.magFilter != -1 ? sampler.magFilter : GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S,
                       sampler.wrapS);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T,
                       sampler.wrapT);
    } else {
        // Default texture parameters, will just be white
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    }

    glGenerateMipmap(GL_TEXTURE_2D);
    glBindTexture(GL_TEXTURE_2D, 0);

    return textureID;
}

void Character::_loadMaterials() {
    for (size_t i = 0; i < _model->materials.size(); ++i) {
        const auto& mat = _model->materials[i];
        Material material;
        material.textureID = 0;
        material.hasTexture = false;

        // metallic on material, will be translated to specular
        if (!mat.pbrMetallicRoughness.baseColorFactor.empty()) {
            material.diffuse = glm::vec3(
                mat.pbrMetallicRoughness.baseColorFactor[0],
                mat.pbrMetallicRoughness.baseColorFactor[1],
                mat.pbrMetallicRoughness.baseColorFactor[2]
            );
        } else {
            material.diffuse = glm::vec3(0.8f);
        }

        // load base color texture
        if (mat.pbrMetallicRoughness.baseColorTexture.index >= 0) {
            material.textureID = _loadTextureFromGLTF(mat.pbrMetallicRoughness.baseColorTexture.index);
            material.hasTexture = (material.textureID != 0);
        }

        // approximate specular from metallic
        float metallic = mat.pbrMetallicRoughness.metallicFactor;
        material.specular = glm::vec3(glm::max(metallic, 0.15f));

        // shine from roughness
        float roughness = mat.pbrMetallicRoughness.roughnessFactor;
        material.shininess = glm::max((1.0f - roughness) * 128.0f, 4.0f);

        _materials.push_back(material);
    }

    // default material
    if (_materials.empty()) {
        Material defaultMat;
        defaultMat.diffuse = glm::vec3(0.8f);
        defaultMat.specular = glm::vec3(0.2f);
        defaultMat.shininess = 32.0f;
        defaultMat.textureID = 0;
        defaultMat.hasTexture = false;
        _materials.push_back(defaultMat);
    }
}

// setup which animation is loaded
void Character::playAnimation(const std::string& animationName) {
    for (size_t i = 0; i < _animations.size(); ++i) {
        if (_animations[i].name == animationName) {
            _animState.currentAnimation = i;
            _animState.currentTime = 0.0f;
            _animState.isPlaying = true;
            return;
        }
    }
    std::cerr << "Animation not found: " << animationName << std::endl;
}

// wrapper for update animation and join transformations
void Character::update(float deltaTime) {
    if (_animState.isPlaying && _animState.currentAnimation >= 0) {
        _updateAnimation(deltaTime);
    }
    _updateJointTransforms();
}

// pathfinding update for enemy AI
void Character::update(float deltaTime, const glm::vec3& targetPosition, float turnSpeed) {
    // move along heading
    _position += _headingVector * _moveSpeed * deltaTime;

    // calculate vector from enemy to target
    glm::vec3 toTarget = targetPosition - _position;
    toTarget.y = 0.0f; // Only turn in horizontal plane

    if (glm::length(toTarget) > 0.01f) {
        glm::vec3 desiredHeading = glm::normalize(toTarget);

        // angle between current heading and new heading
        float currentAngle = atan2(_headingVector.x, _headingVector.z);
        float desiredAngle = atan2(desiredHeading.x, desiredHeading.z);

        // shortest angular difference
        float angleDiff = desiredAngle - currentAngle;

        // normalize
        while (angleDiff > M_PI) angleDiff -= 2.0f * M_PI;
        while (angleDiff < -M_PI) angleDiff += 2.0f * M_PI;

        // one step towards the target
        float maxTurn = turnSpeed * deltaTime;
        float turnAmount = glm::clamp(angleDiff, -maxTurn, maxTurn);

        float newAngle = currentAngle + turnAmount;
        _heading = newAngle;
        _headingVector = glm::normalize(glm::vec3(sin(newAngle), 0.0f, cos(newAngle)));
    }

    // update animation and joints
    if (_animState.isPlaying && _animState.currentAnimation >= 0) {
        _updateAnimation(deltaTime);
    }
    _updateJointTransforms();
}

// update on every frame
void Character::_updateAnimation(float deltaTime) {
    const AnimationClip& anim = _animations[_animState.currentAnimation];

    _animState.currentTime += deltaTime;

    // loop animation if its over the time
    if (_animState.currentTime > anim.duration) {
        _animState.currentTime = fmod(_animState.currentTime, anim.duration);
    }

    // store which joints have animated values to avoid mixing with base transforms
    std::vector<bool> hasTranslation(_joints.size(), false);
    std::vector<bool> hasRotation(_joints.size(), false);
    std::vector<bool> hasScale(_joints.size(), false);
    
    std::vector<glm::vec3> translations(_joints.size());
    std::vector<glm::quat> rotations(_joints.size());
    std::vector<glm::vec3> scales(_joints.size());

    // apply animation data
    for (const auto& channel : anim.channels) {
        if (channel.jointIndex < 0 || channel.jointIndex >= (int)_joints.size()) continue;

        if (channel.type == AnimationClip::Channel::TRANSLATION && !channel.translations.empty()) {
            translations[channel.jointIndex] = _interpolateVec3(channel.times, channel.translations, _animState.currentTime);
            hasTranslation[channel.jointIndex] = true;
        }
        else if (channel.type == AnimationClip::Channel::ROTATION && !channel.rotations.empty()) {
            rotations[channel.jointIndex] = _interpolateQuat(channel.times, channel.rotations, _animState.currentTime);
            hasRotation[channel.jointIndex] = true;
        }
        else if (channel.type == AnimationClip::Channel::SCALE && !channel.scales.empty()) {
            scales[channel.jointIndex] = _interpolateVec3(channel.times, channel.scales, _animState.currentTime);
            hasScale[channel.jointIndex] = true;
        }
    }

    // fill in non-animated transforms with base values
    for (size_t i = 0; i < _joints.size(); ++i) {
        const tinygltf::Node& node = _model->nodes[_model->skins[0].joints[i]];

        if (!hasTranslation[i]) {
            if (!node.translation.empty()) {
                translations[i] = glm::vec3(node.translation[0], node.translation[1], node.translation[2]);
            } else {
                translations[i] = glm::vec3(0.0f);
            }
        }

        if (!hasRotation[i]) {
            if (!node.rotation.empty()) {
                rotations[i] = glm::quat(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            } else {
                rotations[i] = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
            }
        }

        if (!hasScale[i]) {
            if (!node.scale.empty()) {
                scales[i] = glm::vec3(node.scale[0], node.scale[1], node.scale[2]);
            } else {
                scales[i] = glm::vec3(1.0f);
            }
        }
    }

    // get all transforms from the previous transform finders
    for (size_t i = 0; i < _joints.size(); ++i) {
        glm::mat4 T = glm::translate(glm::mat4(1.0f), translations[i]);
        glm::mat4 R = glm::mat4_cast(rotations[i]);
        glm::mat4 S = glm::scale(glm::mat4(1.0f), scales[i]);
        _joints[i].localTransform = T * R * S; // apply
    }
}

void Character::_updateJointTransforms() {
    // set global transforms
    for (size_t i = 0; i < _joints.size(); ++i) {
        if (_joints[i].parentIndex < 0) {
            _joints[i].globalTransform = _joints[i].localTransform;
        } else {
            _joints[i].globalTransform = 
                _joints[_joints[i].parentIndex].globalTransform * 
                _joints[i].localTransform;
        }
        
        // matrix = globalTransform * inverseBindMatrix
        _jointMatrices[i] = _joints[i].globalTransform * _joints[i].inverseBindMatrix;
    }
}

// interp vecs
glm::vec3 Character::_interpolateVec3(const std::vector<float>& times,
                                      const std::vector<glm::vec3>& values,
                                      float time) {
    if (values.empty()) return glm::vec3(0.0f);
    if (values.size() == 1) return values[0];
    
    // clamp time to valid range
    if (time <= times.front()) return values.front();
    if (time >= times.back()) return values.back();
    
    // find keyframes to interpolate between
    for (size_t i = 0; i < times.size() - 1; ++i) {
        if (time >= times[i] && time <= times[i + 1]) {
            float t = (time - times[i]) / (times[i + 1] - times[i]);
            return glm::mix(values[i], values[i + 1], t);
        }
    }
    
    return values.back();
}

// interpolate with slerp instead so it works for quaternions
glm::quat Character::_interpolateQuat(const std::vector<float>& times,
                                      const std::vector<glm::quat>& values,
                                      float time) {
    if (values.empty()) return glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
    if (values.size() == 1) return values[0];
    
    // clamp time to valid range
    if (time <= times.front()) return values.front();
    if (time >= times.back()) return values.back();
    
    // find keyframes to interpolate between
    for (size_t i = 0; i < times.size() - 1; ++i) {
        if (time >= times[i] && time <= times[i + 1]) {
            float t = (time - times[i]) / (times[i + 1] - times[i]);
            return glm::slerp(values[i], values[i + 1], t);
        }
    }
    
    return values.back();
}

// draw function, handles literally every primitive that was loaded from the model
void Character::draw(const glm::mat4& modelMtx, const glm::mat4& viewMtx, const glm::mat4& projMtx) {
    // apply character position
    glm::mat4 charTransform = glm::translate(glm::mat4(1.0f), _position);
    charTransform = glm::rotate(charTransform, _heading, glm::vec3(0.0f, 1.0f, 0.0f));
    charTransform = glm::scale(charTransform, glm::vec3(3.0f, 3.0f, 3.0f));
    glm::mat4 finalModelMtx = modelMtx * charTransform;
    
    // send joint matrices to shader
    if (!_jointMatrices.empty() && _shaderLocations.jointMatrices >= 0) {
        glUniformMatrix4fv(_shaderLocations.jointMatrices, _jointMatrices.size(), 
                          GL_FALSE, &_jointMatrices[0][0][0]);
    }
    
    // draw each primitive
    for (size_t i = 0; i < _primitives.size(); ++i) {
        const Primitive& prim = _primitives[i];

        // set material
        int matIdx = (prim.materialIndex >= 0 && prim.materialIndex < (int)_materials.size())
                     ? prim.materialIndex : 0;
        const Material& mat = _materials[matIdx];

        // set material uniforms
        glProgramUniform3fv(_shaderProgramHandle, _shaderLocations.materialDiffuse, 1, &mat.diffuse[0]);
        glProgramUniform3fv(_shaderProgramHandle, _shaderLocations.materialSpecular, 1, &mat.specular[0]);
        glProgramUniform1f(_shaderProgramHandle, _shaderLocations.materialShininess, mat.shininess);

        // bind texture
        if (mat.hasTexture && mat.textureID != 0) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mat.textureID);
            glUniform1i(glGetUniformLocation(_shaderProgramHandle, "useTexture"), 1);
            glUniform1i(glGetUniformLocation(_shaderProgramHandle, "materialTexture"), 0);
        } else {
            glUniform1i(glGetUniformLocation(_shaderProgramHandle, "useTexture"), 0);
        }

        // send matrices
        _computeAndSendMatrixUniforms(finalModelMtx, viewMtx, projMtx);

        // draw 
        glBindVertexArray(prim.vao);
        if (prim.ibo) {
            glDrawElements(GL_TRIANGLES, prim.indexCount, prim.indexType, nullptr);
        }
        glBindVertexArray(0);

        // unbind texture
        if (mat.hasTexture) {
            glBindTexture(GL_TEXTURE_2D, 0);
        }
    }
}

void Character::_computeAndSendMatrixUniforms(
    const glm::mat4& modelMtx,
    const glm::mat4& viewMtx,
    const glm::mat4& projMtx
) {
    glm::mat4 mvpMtx = projMtx * viewMtx * modelMtx;
    glProgramUniformMatrix4fv(_shaderProgramHandle, _shaderLocations.mvpMtx, 1, GL_FALSE, &mvpMtx[0][0]);

    glm::mat3 normalMtx = glm::mat3(glm::transpose(glm::inverse(modelMtx)));
    glProgramUniformMatrix3fv(_shaderProgramHandle, _shaderLocations.normalMtx, 1, GL_FALSE, &normalMtx[0][0]);

    glProgramUniformMatrix4fv(_shaderProgramHandle, _shaderLocations.modelMtx, 1, GL_FALSE, &modelMtx[0][0]);
}

void Character::moveForward(float amount) {
    _position.x += amount * sin(_heading);
    _position.z += amount * cos(_heading);
}

void Character::moveBackward(float amount) {
    moveForward(-amount);
}

void Character::turnLeft(float angle) {
    _heading += angle;
}

void Character::turnRight(float angle) {
    _heading -= angle;
}

void Character::setPosition(const glm::vec3& position) {
    _position = position;
}

void Character::updateShaderReferences(
    GLuint shaderProgramHandle,
    GLint mvpMtxUniformLocation,
    GLint normalMtxUniformLocation,
    GLint modelMtxUniformLocation,
    GLint materialDiffuseLocation,
    GLint materialSpecularLocation,
    GLint materialShininessLocation
) {
    _shaderProgramHandle = shaderProgramHandle;
    _shaderLocations.mvpMtx = mvpMtxUniformLocation;
    _shaderLocations.normalMtx = normalMtxUniformLocation;
    _shaderLocations.modelMtx = modelMtxUniformLocation;
    _shaderLocations.materialDiffuse = materialDiffuseLocation;
    _shaderLocations.materialSpecular = materialSpecularLocation;
    _shaderLocations.materialShininess = materialShininessLocation;
    _shaderLocations.jointMatrices = glGetUniformLocation(shaderProgramHandle, "jointMatrices");
}