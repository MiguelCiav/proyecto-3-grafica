#version 330 core
layout(location = 0) in vec3 aPos;
layout(location = 1) in vec4 aColor;
layout(location = 2) in vec2 aUV;
layout(location = 3) in float aSpawnTime;

uniform mat4 u_View;
uniform mat4 u_Projection;

out vec4 vColor;
out vec2 vUV;
out float vSpawnTime;

void main() {
    vColor = aColor;
    vUV = aUV;
    vSpawnTime = aSpawnTime;
    gl_Position = u_Projection * u_View * vec4(aPos, 1.0);
}
