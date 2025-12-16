#version 410 core

// uniform inputs
// TODO #E - add uniform
uniform sampler2D textureMap;
uniform bool isTextured;
// varying inputs
// TODO #D - add varying


in vec3 fragNormal;
in vec3 fragPosition;
in vec3 fragSpotlightPosition;
in vec3 fragSpotlightDirection;
in vec3 fragSpotlightColor;
in vec3 fragViewDir;
in vec2 fragTexCoord;

// fragment outputs
out vec4 fragColorOut;

void main() {
    // normalize vectors
    vec3 N = normalize(fragNormal);
    vec3 V = normalize(fragViewDir);

    vec3 pointColor = vec3(0.0f);

    // Spotlight
    vec3 lightToSurfaceDir = normalize(fragPosition-fragSpotlightPosition);
    vec3 spotLightDir = normalize(-lightToSurfaceDir);

    vec4 texColor = texture(textureMap, fragTexCoord);
    vec3 baseColor = texColor.rgb;

    vec3 spotDiffuse = max(dot(N, spotLightDir), 0.0) * baseColor * fragSpotlightColor;

    vec3 spotReflectVec = reflect(-spotLightDir, N);
    float spotSpec = pow(max(dot(V, spotReflectVec),0.0),32);
    vec3 spotSpecular = vec3(0.3) * spotSpec * fragSpotlightColor;

    // spotlight cone
    float innerCut = cos(radians(30.0f));
    float outerCut = cos(radians(35.0f));
    float theta = (dot(spotLightDir, normalize(-fragSpotlightDirection)));
    float intensity = smoothstep(outerCut,innerCut,theta);

    // attenuation
    float distance    = length(fragSpotlightPosition - fragPosition);
    float attenuation = 1.0 / (0.5 + 0.0 * distance + 0.002 * (distance * distance));

    vec3 spotColor = (spotDiffuse+spotSpecular)*intensity*attenuation;

    vec3 finalColor = pointColor + spotColor;

    // TODO #F - get texel
    vec4 texelColor = texture(textureMap, fragTexCoord);
    // TODO #G - set texel
    fragColorOut = texelColor * vec4(finalColor, 1.0f);
}
