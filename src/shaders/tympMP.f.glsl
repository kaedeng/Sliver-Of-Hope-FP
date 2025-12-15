#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D textureMap;
uniform bool isTextured;
// varying inputs
// TODO #D - add varying


layout(location = 0) in vec3 color;
layout(location = 2) in vec2 texPos;
// fragment outputs
out vec4 fragColorOut;

void main() {
    // TODO #F - get texel
    vec4 texelColor;
    if(isTextured){
        texelColor = texture(textureMap, texPos);
    } else {
        texelColor = vec4(1.0f);
    }
    // TODO #G - set texel
    fragColorOut = vec4(color, 1)*texelColor;
}
