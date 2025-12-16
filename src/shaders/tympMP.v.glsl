#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;
uniform mat3 normalMatrix;
uniform mat4 modelMatrix;

uniform vec3 lightDir;
uniform vec3 lightColor;
uniform vec3 lightPosition;             // for the point light stuff
uniform vec3 pointLightColor;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform vec3 ambientLightColor;

uniform vec3 cameraPos;
uniform float shininessAlpha;

uniform vec3 materialColor;


uniform bool isTextured;

// attribute inputs
layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexPos;

// varying outputs
out vec3 fragNormal;
out vec3 fragPosition;
out vec3 fragSpotlightPosition;
out vec3 fragSpotlightDirection;
out vec3 fragSpotlightColor;
out vec3 fragViewDir;
out vec2 fragTexCoord;

void main() {
    gl_Position = mvpMatrix * vec4(vPos, 1.0);
    vec3 worldPos = (modelMatrix * vec4(vPos, 1.0)).xyz;

    vec3 lightVector = normalize(-lightDir);
    vec3 normalizeVNormal = normalize(normalMatrix * vNormal);

    vec3 baseColor;
    if(isTextured){
        baseColor = vec3(1.0f);
    }
    else{
        baseColor = materialColor;
    }

    // Pass data to fragment shader for per-pixel lighting
    fragNormal = normalizeVNormal;
    fragPosition = worldPos;
    fragSpotlightPosition = spotLightPosition;
    fragSpotlightDirection = spotLightDirection;
    fragSpotlightColor = spotLightColor;
    fragViewDir = cameraPos - worldPos;
    fragTexCoord = vTexPos;
}