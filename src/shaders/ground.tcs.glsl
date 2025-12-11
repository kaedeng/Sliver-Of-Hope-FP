#version 410 core

layout(vertices = 4) out;

in vec3 tcPos[];
in vec3 tcNormal[];
in vec2 tcTexCoord[];

out vec3 tePos[];
out vec3 teNormal[];
out vec2 teTexCoord[];

uniform float tessLevel = 32.0; // Tessellation level

void main() {
    // vertex data
    tePos[gl_InvocationID] = tcPos[gl_InvocationID];
    teNormal[gl_InvocationID] = tcNormal[gl_InvocationID];
    teTexCoord[gl_InvocationID] = tcTexCoord[gl_InvocationID];

    // set tessellation levels
    if (gl_InvocationID == 0) {
        // Outer tessellation levels 
        gl_TessLevelOuter[0] = tessLevel;
        gl_TessLevelOuter[1] = tessLevel;
        gl_TessLevelOuter[2] = tessLevel;
        gl_TessLevelOuter[3] = tessLevel;

        // Inner tessellation levels 
        gl_TessLevelInner[0] = tessLevel;
        gl_TessLevelInner[1] = tessLevel;
    }
}
