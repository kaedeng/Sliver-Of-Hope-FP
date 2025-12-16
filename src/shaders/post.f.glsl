#version 410 core

in vec2 fragTexCoord;

uniform sampler2D sceneTexture;

uniform vec2 rOffset;               // Uniform for red channel offset
uniform vec2 gOffset;               // Uniform for green channel offset
uniform vec2 bOffset;               // Uniform for blue channel offset

out vec4 fragColor;

void main() {
    vec4 rValue = texture2D(sceneTexture, fragTexCoord.st + rOffset);
    vec4 gValue = texture2D(sceneTexture, fragTexCoord.st + gOffset);
    vec4 bValue = texture2D(sceneTexture, fragTexCoord.st + bOffset);

    //fragColor = vec4(rValue.r, gValue.g, bValue.b, 1.0f);
    fragColor = vec4(1.0f, 0.0f, 0.0f, 1.0f);
}