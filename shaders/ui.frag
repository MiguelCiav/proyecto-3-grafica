#version 330 core

in vec2 v_TexCoords;
in vec4 v_Color;

uniform sampler2D u_Texture;

out vec4 FragColor;

void main() {
    vec4 texColor = texture(u_Texture, v_TexCoords);
    FragColor = texColor * v_Color;
}
