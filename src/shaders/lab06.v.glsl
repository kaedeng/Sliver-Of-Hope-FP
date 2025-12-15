#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform vec3 cameraPosition;

// attribute inputs
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
// TODO #A - add attribute
layout(location = 2) in vec2 vTexCoord;


// Outputs to fragment shader
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 fragSpotlightPosition;
out vec3 fragSpotlightDirection;
out vec3 fragSpotlightColor;
out vec3 fragViewDir;
out vec2 fragTexCoord;

void main() {
    vec4 position = vec4(vPos, 1.0);
    vec3 normal = vNormal;

    gl_Position = mvpMatrix * position;

    // Calculate world space position
    vec3 worldPos = (modelMatrix * position).xyz;

    // Transform normal to world space
    vec3 normalTransformed = normalize(normalMatrix * normal);

    // Pass data to fragment shader for per-pixel lighting
    fragNormal = normalTransformed;
    fragPosition = worldPos;
    fragSpotlightPosition = spotLightPosition;
    fragSpotlightDirection = spotLightDirection;
    fragSpotlightColor = spotLightColor;
    fragViewDir = cameraPosition - worldPos;
    fragTexCoord = vTexCoord;
}