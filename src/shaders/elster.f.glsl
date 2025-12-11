#version 410 core

// Inputs from vertex shader
in vec3 fragNormal;
in vec3 fragPosition;
in vec3 fragLightDir;
in vec3 fragPointLightPosition;
in vec3 fragPointLightColor;
in vec3 fragSpotlightPosition;
in vec3 fragSpotlightDirection;
in vec3 fragSpotlightColor;
in vec3 fragViewDir;
in vec3 fragLightColor;
in vec2 fragTexCoord;

// Material properties
uniform vec3 materialDiffuse;
uniform vec3 materialSpecular;
uniform float materialShininess;

uniform vec3 ambientLight = vec3(0.2, 0.2, 0.2);

// Texture properties
uniform bool useTexture = false;
uniform sampler2D materialTexture;

// Output color
out vec4 fragColorOut;

void main() {
    // normalize vectors
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(fragLightDir);
    vec3 V = normalize(fragViewDir);

    // get base color from texture or material color
    vec3 baseColor = materialDiffuse;
    if (useTexture) {
        vec4 texColor = texture(materialTexture, fragTexCoord);
        baseColor = texColor.rgb * materialDiffuse;
    }


    // phongs steps
    vec3 ambient = ambientLight * baseColor;

    float diffuseIntensity = max(dot(N, L), 0.0);
    vec3 diffuse = diffuseIntensity * fragLightColor * baseColor;

    vec3 specular = vec3(0.0);
    if (diffuseIntensity > 0.0) {
        vec3 R = reflect(-L, N);
        float specularIntensity = pow(max(dot(R, V), 0.0), materialShininess);
        specular = specularIntensity * fragLightColor * materialSpecular;
    }

    // add all for phongs
    vec3 dirColor = diffuse + specular;

        // POINT LIGHT
        vec3 posLightVec = normalize(fragPointLightPosition - fragPosition);

        // TODO #F: perform diffuse calculation
        vec3 posDiffuse = max(dot(N, posLightVec), 0.0) *fragPointLightColor * baseColor;

        // Specular
        vec3 posReflectVec = reflect(-posLightVec, N);
        float posSpec = pow(max(dot(V, posReflectVec), 0.0), 32);
        vec3 posSpecular = vec3(0.5, 0.5, 0.5) * posSpec * fragPointLightColor;

        // attenuation
            float pointDistance    = length(fragPointLightPosition - fragPosition);
            float pointAttenuation = 1.0 / (1.0 + 0.09 * pointDistance + 0.032 * (pointDistance * pointDistance));

        // TODO #G: assign the color for this vertex
        vec3 pointColor = (posDiffuse + posSpecular)*pointAttenuation;

        // SPOTLIGHT
        vec3 lightToSurfaceDir = normalize(fragPosition-fragSpotlightPosition);
        vec3 spotLightDir = normalize(-lightToSurfaceDir);

        vec3 spotDiffuse = max(dot(N, spotLightDir), 0.0) * baseColor * fragSpotlightColor;

        vec3 spotReflectVec = reflect(-spotLightDir, N);
        float spotSpec = pow(max(dot(V, spotReflectVec),0.0),32);
        vec3 spotSpecular = vec3(0.5,0.5,0.5) * spotSpec * fragSpotlightColor;

        // spotlight cone
        float innerCut = cos(radians(30.0f));
        float outerCut = cos(radians(35.0f));
        float theta = (dot(spotLightDir, normalize(-fragSpotlightDirection)));
        float intensity = smoothstep(outerCut,innerCut,theta);

        // attenuation
        float distance    = length(fragSpotlightPosition - fragPosition);
        float attenuation = 1.0 / (1.0 + 0.09 * distance + 0.032 * (distance * distance));

        vec3 spotColor = (spotDiffuse+3*spotSpecular)*intensity*attenuation;

    vec3 finalColor = ambient + dirColor + 1.5f*pointColor + 1.5f*spotColor;

    // return
    fragColorOut = vec4(finalColor, 1.0);
}
