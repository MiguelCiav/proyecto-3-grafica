#version 330 core
out vec4 FragColor;

in vec3 TexCoords;

uniform samplerCube u_Skybox;
uniform vec3 u_SkyTint = vec3(1.0);

void main()
{
    FragColor = vec4(texture(u_Skybox, TexCoords).rgb * u_SkyTint, 1.0);
}
