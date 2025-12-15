#ifndef TYMPANIUS_SHADER_ATTRIBUTE_LOCATIONS_HPP
#define TYMPANIUS_SHADER_ATTRIBUTE_LOCATIONS_HPP

#include <glad/gl.h>

struct TympaniusShaderAttributeLocations {
    /// \desc vertex position location
    GLint vPos;
    // TODO #2: add new attributes
    GLint vNormal;
    // TODO #10 - texture coordinate
    GLint texCoord;
};

#endif