#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D textureMap;
// varying inputs
// TODO #D - add varying
in vec2 texPos;
// fragment outputs
out vec4 fragColorOut;

void main() {
    // TODO #F - get texel
    vec4 texelColor = texture(textureMap, texPos);
    // TODO #G - set texel
    fragColorOut = texelColor;
}