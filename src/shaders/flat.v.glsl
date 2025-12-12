#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;

// attribute inputs
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;

// varying outputs
out vec3 fragNormal;
out vec3 fragPosition;

void main() {
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    // Transform normal to world space for lighting
    fragNormal = normalize(normalMatrix * vNormal);

    // Transform position to world space
    fragPosition = vec3(modelMatrix * vec4(vPos, 1.0));
}
