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

// Bezier curve evaluation for 4 control points
vec3 bezierCurve(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float t2 = t * t;
    float t3 = t2 * t;
    float mt = 1.0 - t;
    float mt2 = mt * mt;
    float mt3 = mt2 * mt;

    return mt3 * p0 + 3.0 * mt2 * t * p1 + 3.0 * mt * t2 * p2 + t3 * p3;
}

// Derivative of Bezier curve for computing tangents
vec3 bezierCurveDerivative(vec3 p0, vec3 p1, vec3 p2, vec3 p3, float t) {
    float t2 = t * t;
    float mt = 1.0 - t;
    float mt2 = mt * mt;

    return 3.0 * mt2 * (p1 - p0) + 6.0 * mt * t * (p2 - p1) + 3.0 * t2 * (p3 - p2);
}

// get control point
vec3 getControlPoint(vec3 p00, vec3 p10, vec3 p01, vec3 p11, int i, int j) {
    // Row 0 (bottom edge)
    if (i == 0 && j == 0) return p00;
    if (i == 0 && j == 1) return mix(p00, p10, 0.333) + vec3(0, hillHeight * 0.1, 0);
    if (i == 0 && j == 2) return mix(p00, p10, 0.667) + vec3(0, hillHeight * 0.1, 0);
    if (i == 0 && j == 3) return p10;

    // Row 1 (first interior row)
    if (i == 1 && j == 0) return mix(p00, p01, 0.333) + vec3(0, hillHeight * 0.1, 0);
    if (i == 1 && j == 1) return mix(mix(p00, p10, 0.333), mix(p01, p11, 0.333), 0.333) + vec3(0, hillHeight * 0.6, 0);
    if (i == 1 && j == 2) return mix(mix(p00, p10, 0.667), mix(p01, p11, 0.667), 0.333) + vec3(0, hillHeight * 0.6, 0);
    if (i == 1 && j == 3) return mix(p10, p11, 0.333) + vec3(0, hillHeight * 0.1, 0);

    // Row 2 (second interior row)
    if (i == 2 && j == 0) return mix(p00, p01, 0.667) + vec3(0, hillHeight * 0.1, 0);
    if (i == 2 && j == 1) return mix(mix(p00, p10, 0.333), mix(p01, p11, 0.333), 0.667) + vec3(0, hillHeight * 0.6, 0);
    if (i == 2 && j == 2) return mix(mix(p00, p10, 0.667), mix(p01, p11, 0.667), 0.667) + vec3(0, hillHeight * 0.6, 0);
    if (i == 2 && j == 3) return mix(p10, p11, 0.667) + vec3(0, hillHeight * 0.1, 0);

    // Row 3 (top edge)
    if (i == 3 && j == 0) return p01;
    if (i == 3 && j == 1) return mix(p01, p11, 0.333) + vec3(0, hillHeight * 0.1, 0);
    if (i == 3 && j == 2) return mix(p01, p11, 0.667) + vec3(0, hillHeight * 0.1, 0);
    if (i == 3 && j == 3) return p11;

    return vec3(0.0); // fail case
}

// Bezier interpolation
vec3 bezierInterpolate(float u, float v) {
    // 4 corner positions from the patch
    vec3 p00 = tePos[0];
    vec3 p10 = tePos[1];
    vec3 p01 = tePos[2];
    vec3 p11 = tePos[3];

    // evaluate 4 Bezier curves along u direction
    vec3 curve0 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 0, 0),
        getControlPoint(p00, p10, p01, p11, 0, 1),
        getControlPoint(p00, p10, p01, p11, 0, 2),
        getControlPoint(p00, p10, p01, p11, 0, 3),
        u
    );

    vec3 curve1 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 1, 0),
        getControlPoint(p00, p10, p01, p11, 1, 1),
        getControlPoint(p00, p10, p01, p11, 1, 2),
        getControlPoint(p00, p10, p01, p11, 1, 3),
        u
    );

    vec3 curve2 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 2, 0),
        getControlPoint(p00, p10, p01, p11, 2, 1),
        getControlPoint(p00, p10, p01, p11, 2, 2),
        getControlPoint(p00, p10, p01, p11, 2, 3),
        u
    );

    vec3 curve3 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 3, 0),
        getControlPoint(p00, p10, p01, p11, 3, 1),
        getControlPoint(p00, p10, p01, p11, 3, 2),
        getControlPoint(p00, p10, p01, p11, 3, 3),
        u
    );

    // evaluate final curve through those 4 results along v direction
    return bezierCurve(curve0, curve1, curve2, curve3, v);
}

// Compute normal from Bezier surface tangents
vec3 computeNormal(float u, float v) {
    vec3 p00 = tePos[0];
    vec3 p10 = tePos[1];
    vec3 p01 = tePos[2];
    vec3 p11 = tePos[3];

    // evaluate derivative of 4 curves along u direction
    vec3 dCurve0_du = bezierCurveDerivative(
        getControlPoint(p00, p10, p01, p11, 0, 0),
        getControlPoint(p00, p10, p01, p11, 0, 1),
        getControlPoint(p00, p10, p01, p11, 0, 2),
        getControlPoint(p00, p10, p01, p11, 0, 3),
        u
    );

    vec3 dCurve1_du = bezierCurveDerivative(
        getControlPoint(p00, p10, p01, p11, 1, 0),
        getControlPoint(p00, p10, p01, p11, 1, 1),
        getControlPoint(p00, p10, p01, p11, 1, 2),
        getControlPoint(p00, p10, p01, p11, 1, 3),
        u
    );

    vec3 dCurve2_du = bezierCurveDerivative(
        getControlPoint(p00, p10, p01, p11, 2, 0),
        getControlPoint(p00, p10, p01, p11, 2, 1),
        getControlPoint(p00, p10, p01, p11, 2, 2),
        getControlPoint(p00, p10, p01, p11, 2, 3),
        u
    );

    vec3 dCurve3_du = bezierCurveDerivative(
        getControlPoint(p00, p10, p01, p11, 3, 0),
        getControlPoint(p00, p10, p01, p11, 3, 1),
        getControlPoint(p00, p10, p01, p11, 3, 2),
        getControlPoint(p00, p10, p01, p11, 3, 3),
        u
    );

    // evaluate curve through derivatives along v direction
    vec3 tangentU = bezierCurve(dCurve0_du, dCurve1_du, dCurve2_du, dCurve3_du, v);

    // evaluate 4 curves along u direction (same as position)
    vec3 curve0 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 0, 0),
        getControlPoint(p00, p10, p01, p11, 0, 1),
        getControlPoint(p00, p10, p01, p11, 0, 2),
        getControlPoint(p00, p10, p01, p11, 0, 3),
        u
    );

    vec3 curve1 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 1, 0),
        getControlPoint(p00, p10, p01, p11, 1, 1),
        getControlPoint(p00, p10, p01, p11, 1, 2),
        getControlPoint(p00, p10, p01, p11, 1, 3),
        u
    );

    vec3 curve2 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 2, 0),
        getControlPoint(p00, p10, p01, p11, 2, 1),
        getControlPoint(p00, p10, p01, p11, 2, 2),
        getControlPoint(p00, p10, p01, p11, 2, 3),
        u
    );

    vec3 curve3 = bezierCurve(
        getControlPoint(p00, p10, p01, p11, 3, 0),
        getControlPoint(p00, p10, p01, p11, 3, 1),
        getControlPoint(p00, p10, p01, p11, 3, 2),
        getControlPoint(p00, p10, p01, p11, 3, 3),
        u
    );

    // evaluate derivative of curve through results along v direction
    vec3 tangentV = bezierCurveDerivative(curve0, curve1, curve2, curve3, v);

    // Normal is cross product of tangents
    return normalize(cross(tangentU, tangentV));
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
    vec3 localNormal = computeNormal(u, v);

    // to world space
    worldPos = (modelMatrix * vec4(localPos, 1.0)).xyz;
    fragNormal = normalize(normalMatrix * localNormal);

    // to clip space
    gl_Position = mvpMatrix * vec4(localPos, 1.0);
}
