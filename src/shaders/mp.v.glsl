#version 410 core

// uniform inputs
uniform mat4 mvpMatrix;                 // the precomputed Model-View-Projection Matrix
uniform mat4 modelMatrix;
// TODO #D: add normal matrix
uniform mat3 normalMatrix;

// TODO #A: add light uniforms
uniform vec3 lightDirection;             // The light's position (used as direction) in World Space
uniform vec3 lightPosition;             // for the point light stuff
uniform vec3 pointLightColor;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform vec3 lightColor;
uniform vec3 cameraPosition;

uniform vec3 materialColor;             // the material color for our vertex (& whole object)

// attribute inputs
layout(location = 0) in vec3 vPos;      // the position of this specific vertex in object space
// TODO #C: add vertex normal
layout(location = 1) in vec3 vNormal;

// varying outputs
layout(location = 0) out vec3 color;    // color to apply to this vertex

void main() {
    // DIRECTIION LIGHT
    // transform & output the vertex in clip space
    gl_Position = mvpMatrix * vec4(vPos, 1.0);

    vec3 worldPos = (modelMatrix * vec4(vPos, 1.0)).xyz;

    // TODO #B: compute Light vector
    vec3 lightVec = normalize(lightDirection);

    // TODO #E: transform normal vector
    vec3 normal = normalize(normalMatrix * vNormal);

    // TODO #F: perform diffuse calculation
    vec3 diffuse = lightColor * materialColor * max(dot(normal, lightVec), 0.0);

    // Calculate ambient so that the shadow is not completely black
    vec3 ambient = vec3(0.71, 0.54, 0.7) * materialColor;

    // Specular
    vec3 viewVec = normalize(cameraPosition - worldPos);
    vec3 reflectVec = reflect(-lightVec, normal);
    float spec = pow(max(dot(viewVec, reflectVec), 0.0), 32);
    vec3 specular = vec3(0.5, 0.5, 0.5) * spec;

    // TODO #G: assign the color for this vertex
    vec3 dirColor = diffuse + specular;

    // POINT LIGHT
    vec3 posLightVec = normalize(lightPosition - worldPos);

    // TODO #E: transform normal vector
    vec3 posNormal = normalize(normalMatrix * vNormal);

    // TODO #F: perform diffuse calculation
    vec3 posDiffuse = max(dot(posNormal, posLightVec), 0.0) *pointLightColor * materialColor;

    // Specular
    vec3 posReflectVec = reflect(-posLightVec, posNormal);
    float posSpec = pow(max(dot(viewVec, posReflectVec), 0.0), 32);
    vec3 posSpecular = vec3(0.5, 0.5, 0.5) * posSpec * pointLightColor;

    // attenuation
        float pointDistance    = length(lightPosition - worldPos);
        float pointAttenuation = 1.0 / (1.0 + 0.09 * pointDistance + 0.032 * (pointDistance * pointDistance));

    // TODO #G: assign the color for this vertex
    vec3 pointColor = (posDiffuse + posSpecular)*pointAttenuation;

    // SPOTLIGHT
    vec3 lightToSurfaceDir = normalize(worldPos-spotLightPosition);
    vec3 spotLightDir = normalize(-lightToSurfaceDir);

    vec3 spotDiffuse = max(dot(normal, spotLightDir), 0.0) * materialColor * spotLightColor;

    vec3 spotReflectVec = reflect(-spotLightDir, normal);
    float spotSpec = pow(max(dot(viewVec, spotReflectVec),0.0),32);
    vec3 spotSpecular = vec3(0.5,0.5,0.5) * spotSpec * spotLightColor;

    // spotlight cone
    float innerCut = cos(radians(30.0f));
    float outerCut = cos(radians(35.0f));
    float theta = (dot(spotLightDir, normalize(-spotLightDirection)));
    float intensity = smoothstep(outerCut,innerCut,theta);

    // attenuation
    float distance    = length(spotLightPosition - worldPos);
    float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

    vec3 spotColor = (spotDiffuse+spotSpecular)*intensity*attenuation;


    color = ambient + dirColor + pointColor + spotColor;
}
