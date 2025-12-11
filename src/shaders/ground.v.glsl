#version 410 core



layout(location = 0) in vec3 vPos;
layout(location = 1) in vec3 vNormal;
layout(location = 2) in vec2 vTexCoord;

out vec3 tcPos;
out vec3 tcNormal;
out vec2 tcTexCoord;

// vs for tessellation pass-through

void main() {
    tcPos = vPos;
    tcNormal = vNormal;
    tcTexCoord = vTexCoord;
}
