#version 410 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec2 texCoord;

out vec2 fragTexCoord;

void main() {
    // Positions are already in NDC (-1 to 1), pass through directly
    gl_Position = vec4(vPos, 1.0);
    fragTexCoord = texCoord;
}
