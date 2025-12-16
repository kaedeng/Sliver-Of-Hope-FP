#version 410 core

in vec2 fragTexCoord;

uniform sampler2D sceneTexture;

uniform vec2 rOffset;               // Uniform for red channel offset
uniform vec2 gOffset;               // Uniform for green channel offset
uniform vec2 bOffset;               // Uniform for blue channel offset
uniform float rNoise;
uniform float gNoise;
uniform float bNoise;

out vec4 fragColor;

void main() {
    // Sample each color channel with its offset for chromatic aberration
    float r = texture(sceneTexture, fragTexCoord + rOffset).r;
    float g = texture(sceneTexture, fragTexCoord + gOffset).g;
    float b = texture(sceneTexture, fragTexCoord + bOffset).b;

    fragColor = vec4(vec3(r, g, b)*(1.0f-rNoise*15), 1.0);
}