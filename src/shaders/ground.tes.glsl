#version 410 core

layout(quads, equal_spacing, ccw) in;

in vec3 tePos[];
in vec3 teNormal[];
in vec2 teTexCoord[];

out vec3 worldPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform float hillHeight = 56.25; // Maximum height of the mountain

// Bezier interpolation
vec3 bezierInterpolate(float u, float v) {
    // 4 corner positions from the patch
    vec3 p00 = tePos[0];
    vec3 p10 = tePos[1];
    vec3 p01 = tePos[2];
    vec3 p11 = tePos[3];

    return vec3(mix(p00.x, p11.x, u), hillHeight, mix(p00.z, p11.z, v));
}

void main() {
    // tess coordinates
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // texture coordinates
    vec2 texCoord0 = mix(teTexCoord[0], teTexCoord[1], u);
    vec2 texCoord1 = mix(teTexCoord[2], teTexCoord[3], u);
    fragTexCoord = mix(texCoord0, texCoord1, v);

    // position using Bezier surface
    vec3 localPos = bezierInterpolate(u, v);

    // normal from surface
    vec3 localNormal = vec3(0, 1, 0);

    // to world space
    worldPos = (modelMatrix * vec4(localPos, 1.0)).xyz;
    fragNormal = normalize(normalMatrix * localNormal);

    // to clip space
    gl_Position = mvpMatrix * vec4(localPos, 1.0);
}
