#version 410 core

// uniform inputs
uniform sampler2D textureMap;
uniform vec3 materialColor;
uniform bool isTextured;

in vec3 fragNormal;
in vec3 fragPosition;
in vec3 fragSpotlightPosition;
in vec3 fragSpotlightDirection;
in vec3 fragSpotlightColor;
in vec3 fragViewDir;
in vec2 fragTexCoord;

// red glowing eyes on enemies
const int NUM_EYE_LIGHTS = 8;
uniform vec3 eyeLightPositions[NUM_EYE_LIGHTS];
uniform vec3 eyeLightColor = vec3(1.0, 0.0, 0.0); // Red glow

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
    
    vec3 baseColor;
    if(isTextured){
        vec4 texColor = texture(textureMap, fragTexCoord);
        baseColor = texColor.rgb;
    }
    else{
        baseColor = materialColor;
    }

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

    // red glowing eyes on enemies
    vec3 eyeLightTotal = vec3(0.0); // color accumulator for just pointlights
    for (int i = 0; i < NUM_EYE_LIGHTS; i++) {
        vec3 eyeLightDir = normalize(eyeLightPositions[i] - fragPosition);

        // diffuse
        vec3 eyeDiffuse = max(dot(N, eyeLightDir), 0.0) * eyeLightColor * baseColor;

        // specular
        vec3 eyeReflectVec = reflect(-eyeLightDir, N);
        float eyeSpec = pow(max(dot(V, eyeReflectVec), 0.0), 32.0);
        vec3 eyeSpecular = vec3(0.5) * eyeSpec * eyeLightColor;

        // attenuation
        float eyeDistance = length(eyeLightPositions[i] - fragPosition);
        float eyeAttenuation = 1.0 / (1.0 + 0.5 * eyeDistance + 0.3 * (eyeDistance * eyeDistance));

        eyeLightTotal += (eyeDiffuse + eyeSpecular) * eyeAttenuation;
    }

    vec3 finalColor = pointColor + spotColor + eyeLightTotal;

    if(isTextured){
        vec4 texelColor = texture(textureMap, fragTexCoord);
        fragColorOut = texelColor * vec4(finalColor, 1.0f);
    }
    else{
        fragColorOut = vec4(finalColor, 1.0f);
    }
}