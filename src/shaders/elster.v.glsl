#version 410 core

layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in ivec4 vJoints;
layout(location = 3) in vec4 vWeights;
layout(location = 4) in vec2 vTexCoord;

uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

const int MAX_JOINTS = 100;
uniform mat4 jointMatrices[MAX_JOINTS];
uniform bool useSkinning = false;

uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 lightPosition;             // for the point light stuff
uniform vec3 pointLightColor;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform vec3 cameraPosition;

// Outputs to fragment shader
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 fragLightDir;
out vec3 fragPointLightPosition;
out vec3 fragPointLightColor;
out vec3 fragSpotlightPosition;
out vec3 fragSpotlightDirection;
out vec3 fragSpotlightColor;
out vec3 fragViewDir;
out vec3 fragLightColor;
out vec2 fragTexCoord;

void main() {
    vec4 position = vec4(vPos, 1.0);
    vec3 normal = vNormal;

    // handles character skinning
    if (useSkinning && vWeights.x > 0.0) {
        mat4 skinMatrix =
            vWeights.x * jointMatrices[vJoints.x] +
            vWeights.y * jointMatrices[vJoints.y] +
            vWeights.z * jointMatrices[vJoints.z] +
            vWeights.w * jointMatrices[vJoints.w];

        position = skinMatrix * position;
        // Use inverse transpose for proper normal transformation
        mat3 skinNormalMatrix = transpose(inverse(mat3(skinMatrix)));
        normal = skinNormalMatrix * normal;
    }

    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * position;

    // Calculate world space position
    vec3 worldPos = (modelMatrix * position).xyz;

    // Transform normal to world space
    vec3 normalTransformed = normalize(normalMatrix * normal);

    // Pass data to fragment shader for per-pixel lighting
    fragNormal = normalTransformed;
    fragPosition = worldPos;
    fragLightDir = lightDirection;
    fragPointLightPosition = lightPosition;
    fragPointLightColor = pointLightColor;
    fragSpotlightPosition = spotLightPosition;
    fragSpotlightDirection = spotLightDirection;
    fragSpotlightColor = spotLightColor;
    fragViewDir = cameraPosition - worldPos;
    fragLightColor = lightColor;
    fragTexCoord = vTexCoord;

    // add back when using with tympanius
    // fragTexCoord = vec2(vTexCoord.x, 1.0 - vTexCoord.y);
}
