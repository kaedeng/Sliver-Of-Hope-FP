#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;

// attribute inputs
layout(location = 0) in vec3 vPos;
// TODO #A - add attribute
layout(location = 2) in vec2 vTexPos;
// varying outputs
// TODO #B - add varying
out vec2 texPos;
void main() {
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    // TODO #C - set varying
    texPos = vTexPos;
}