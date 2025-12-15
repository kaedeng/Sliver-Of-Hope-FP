#version 410 core

layout(quads, equal_spacing, ccw) in;

in vec3 tePos[];
// teNormal and teTexCoord are not used for position/normal calculation here
// We'll keep them in to maintain the original structure
in vec3 teNormal[];
in vec2 teTexCoord[];

out vec3 worldPos;
out vec3 fragNormal;
out vec2 fragTexCoord;

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalMatrix;
uniform float hillHeight = 56.25;

vec3 bilinearInterpolate(float u, float v) {
    vec3 p00 = tePos[0];
    vec3 p10 = tePos[1];
    vec3 p01 = tePos[2];
    vec3 p11 = tePos[3];

    vec3 p_u0 = mix(p00, p10, u);
    vec3 p_u1 = mix(p01, p11, u);
    
    vec3 p_uv = mix(p_u0, p_u1, v);
    p_uv.y = max(p_uv.y, hillHeight);

    return p_uv;
}

vec3 calculateNormal(float u, float v) {
    float delta = 0.001;
    
    vec3 pos_u_plus  = bilinearInterpolate(min(u + delta, 1.0), v);
    vec3 pos_u_minus = bilinearInterpolate(max(u - delta, 0.0), v);
    vec3 pos_v_plus  = bilinearInterpolate(u, min(v + delta, 1.0));
    vec3 pos_v_minus = bilinearInterpolate(u, max(v - delta, 0.0));
    
    vec3 tangent_u = pos_u_plus - pos_u_minus;
    vec3 tangent_v = pos_v_plus - pos_v_minus;
    
    return normalize(cross(tangent_v, tangent_u));
}

void main() {
    // tess coordinates
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    // texture coordinates
    vec2 texCoord0 = mix(teTexCoord[0], teTexCoord[1], u);
    vec2 texCoord1 = mix(teTexCoord[2], teTexCoord[3], u);
    fragTexCoord = mix(texCoord0, texCoord1, v);

    // position using Bilinear surface
    vec3 localPos = bilinearInterpolate(u, v);

    // normal from surface calculation
    vec3 localNormal = calculateNormal(u, v);
    
    // to world space
    worldPos = (modelMatrix * vec4(localPos, 1.0)).xyz;
    fragNormal = normalize(normalMatrix * localNormal);

    // to clip space
    gl_Position = mvpMatrix * vec4(localPos, 1.0);
}