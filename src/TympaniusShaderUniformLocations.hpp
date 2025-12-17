#ifndef TYMPANIUS_SHADER_UNIFORM_LOCATIONS_HPP
#define TYMPANIUS_SHADER_UNIFORM_LOCATIONS_HPP

#include <glad/gl.h>

struct TympaniusShaderUniformLocations {
    /// \desc precomputed MVP matrix location
    GLint mvpMatrix;
    GLint normalMatrix;
    GLint modelMatrix;

    GLint lightDir;
    GLint lightColor;
    GLint lightPosition;
    GLint pointLightColor;
    GLint spotLightPosition;
    GLint spotLightDirection;
    GLint spotLightColor;
    GLint ambientLightColor;
    
    GLint cameraPos;
    GLint shininessAlpha;
    /// \desc material diffuse color location
    GLint materialColor;

    GLint isTextured;
    GLint textureMap;

    // Eye lights
    GLint eyeLightPositions[8];
    GLint eyeLightColor;
};

#endif