#version 330 core
layout (location = 0) in vec3 a_Position;

out vec3 TexCoords;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    TexCoords = a_Position;
    mat4 viewNoTranslation = mat4(mat3(u_View));
    vec4 pos = u_Projection * viewNoTranslation * vec4(a_Position, 1.0);
    gl_Position = pos.xyww;
}
