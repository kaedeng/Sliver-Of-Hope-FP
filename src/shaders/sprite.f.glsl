#version 410 core



in vec2 texCoord;

uniform sampler2D spriteTexture;

out vec4 fragColor;

// billboarded sprites frag shader

void main() {
    vec4 texColor = texture(spriteTexture, texCoord);

    // Discard fully transparent pixels
    if (texColor.a < 0.1) {
        discard;
    }

    fragColor = texColor;
}
