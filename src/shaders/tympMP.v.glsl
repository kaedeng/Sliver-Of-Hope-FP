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
layout(location = 0) out vec3 color;    // color to apply to this vertex
layout(location = 2) out vec2 texPos;

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

    vec3 lightDiffuse = lightColor*baseColor*max(dot(lightVector, normalizeVNormal), 0);

    vec3 reflectionVector = -lightDir+2*dot(normalizeVNormal, lightDir)*normalizeVNormal;
    vec3 viewVector = normalize(cameraPos-worldPos);
    vec3 lightSpecular = lightColor*baseColor*pow(max(dot(reflectionVector, viewVector), 0), shininessAlpha);
    
    vec3 lightAmbient = ambientLightColor*baseColor;

    vec3 dirColor = lightDiffuse + lightSpecular;
    
    color = lightAmbient + dirColor;

    texPos = vTexPos;
}