#version 410 core

// uniform inputs
uniform vec3 lightDirection;
uniform vec3 materialColor;

// varying inputs
in vec3 fragNormal;
in vec3 fragPosition;

// outputs
out vec4 fragColorOut;

void main() {
    vec3 normal = normalize(fragNormal);

    // simple directional lighting
    vec3 lightDir = normalize(-lightDirection);
    float diffuse = max(dot(normal, lightDir), 0.0);

    float flatDiffuse;
    if (diffuse > 0.75) {
        flatDiffuse = 1.0;
    } else if (diffuse > 0.5) {
        flatDiffuse = 0.7;
    } else if (diffuse > 0.25) {
        flatDiffuse = 0.4;
    } else {
        flatDiffuse = 0.2;
    }

    // Apply flat lighting to material color
    vec3 finalColor = materialColor * flatDiffuse;

    fragColorOut = vec4(finalColor, 1.0);
}
