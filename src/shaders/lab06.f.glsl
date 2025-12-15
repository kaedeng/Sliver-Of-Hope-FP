#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D texMap;

// varying inputs
// TODO #D - add varying
layout(location = 2) in vec2 fragTex;

// fragment outputs
out vec4 fragColorOut;

void main() {
    // TODO #F - get texel
    vec4 theTexel = texture(texMap, fragTex);

    // TODO #G - set texel
    fragColorOut = vec4(fragTex,0.0,1.0);
}