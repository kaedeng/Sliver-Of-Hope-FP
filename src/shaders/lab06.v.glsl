#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;

// attribute inputs
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
// TODO #A - add attribute
layout(location = 2) in vec2 vTexCoord;

// varying outputs
// TODO #B - add varying
layout(location = 2) out vec2 fragTex;

void main() {
vec3 unused = vNormal;
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    // TODO #C - set varying
    fragTex = vTexCoord;
}