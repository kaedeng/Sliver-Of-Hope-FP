#version 410 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 texCoord;

uniform mat4 mvpMatrix;

out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix * vec4(vPos, 1.0);
    fragTexCoord = texCoord;
}
