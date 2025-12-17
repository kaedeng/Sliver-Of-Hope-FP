#version 410 core

// Fragment shader for textured ground with lighting

in vec3 worldPos;
in vec3 fragNormal;
in vec2 fragTexCoord;

out vec4 fragColorOut;

uniform sampler2D groundTexture;
uniform vec3 lightDirection;
uniform vec3 lightColor;
uniform vec3 lightPosition;
uniform vec3 pointLightColor;
uniform vec3 spotLightPosition;
uniform vec3 spotLightDirection;
uniform vec3 spotLightColor;
uniform vec3 cameraPosition;

const int NUM_EYE_LIGHTS = 8;
uniform vec3 eyeLightPositions[NUM_EYE_LIGHTS];
uniform vec3 eyeLightColor = vec3(1.0, 0.0, 0.0);

void main() {
    // Sample texture
    vec4 texColor = texture(groundTexture, fragTexCoord);

    // Normalize interpolated normal
    vec3 normal = normalize(fragNormal);

    // DIRECTIONAL LIGHT
    vec3 lightVec = normalize(lightDirection);
    vec3 diffuse = lightColor * texColor.rgb * max(dot(normal, lightVec), 0.0);

    // Specular
    vec3 viewVec = normalize(cameraPosition - worldPos);
    vec3 reflectVec = reflect(-lightVec, normal);
    float spec = pow(max(dot(viewVec, reflectVec), 0.0), 32.0);
    vec3 specular = vec3(0.0) * spec;

    vec3 dirColor = diffuse + specular;

    // POINT LIGHT
    vec3 posLightVec = normalize(lightPosition - worldPos);
    vec3 posDiffuse = max(dot(normal, posLightVec), 0.0) * pointLightColor * texColor.rgb;

    vec3 posReflectVec = reflect(-posLightVec, normal);
    float posSpec = pow(max(dot(viewVec, posReflectVec), 0.0), 32.0);
    vec3 posSpecular = vec3(0.3) * posSpec * pointLightColor;

    // Attenuation
    float pointDistance = length(lightPosition - worldPos);
    float pointAttenuation = 1.0 / (1.0 + 0.09 * pointDistance + 0.032 * (pointDistance * pointDistance));

    vec3 pointColor = (posDiffuse + posSpecular) * pointAttenuation;

    // SPOTLIGHT
    vec3 lightToSurfaceDir = normalize(worldPos - spotLightPosition);
    vec3 spotLightDir = normalize(-lightToSurfaceDir);

    vec3 spotDiffuse = max(dot(normal, spotLightDir), 0.0) * texColor.rgb * spotLightColor;

    vec3 spotReflectVec = reflect(-spotLightDir, normal);
    float spotSpec = pow(max(dot(viewVec, spotReflectVec), 0.0), 32.0);
    vec3 spotSpecular = vec3(0.3) * spotSpec * spotLightColor;

    // Spotlight cone
    float innerCut = cos(radians(25.0));
    float outerCut = cos(radians(35.0));
    float theta = dot(spotLightDir, normalize(-spotLightDirection));
    float intensity = smoothstep(outerCut, innerCut, theta) * 0.75;

    // Attenuation
    float distance = length(spotLightPosition - worldPos);
    float attenuation = 1.0 / (0.5 + 0.0 * distance + 0.0015 * (distance * distance));

    vec3 spotColor = (spotDiffuse + spotSpecular) * intensity * attenuation;

    // Ambient
    vec3 ambient = vec3(0.0, 0.0, 0.0) * texColor.rgb;

    // red glowing eyes on enemies
    vec3 eyeLightTotal = vec3(0.0);
    for (int i = 0; i < NUM_EYE_LIGHTS; i++) {
        vec3 eyeLightDir = normalize(eyeLightPositions[i] - worldPos);

        // diffuse
        vec3 eyeDiffuse = max(dot(normal, eyeLightDir), 0.0) * eyeLightColor * texColor.rgb;

        // specular
        vec3 eyeReflectVec = reflect(-eyeLightDir, normal);
        float eyeSpec = pow(max(dot(viewVec, eyeReflectVec), 0.0), 32.0);
        vec3 eyeSpecular = vec3(0.5) * eyeSpec * eyeLightColor;

        // attenuation
        float eyeDistance = length(eyeLightPositions[i] - worldPos);
        float eyeAttenuation = 1.0 / (1.0 + 0.5 * eyeDistance + 0.3 * (eyeDistance * eyeDistance));

        eyeLightTotal += (eyeDiffuse + eyeSpecular) * eyeAttenuation;
    }

    // Combine all lighting
    vec3 finalColor = ambient + dirColor + pointColor + spotColor + eyeLightTotal;

    fragColorOut = vec4(finalColor, texColor.a);
}
