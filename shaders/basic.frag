#version 330 core
in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoords;

out vec4 FragColor;

uniform sampler2D u_Texture;
uniform bool u_UseTexture;
uniform vec4 u_Color;

void main() {
    vec4 baseColor = u_UseTexture ? texture(u_Texture, v_TexCoords) : u_Color;

    // Iluminación direccional suave por defecto
    vec3 lightDir = normalize(vec3(0.5, 1.0, 0.4));
    vec3 norm = normalize(v_Normal);
    float diff = max(dot(norm, lightDir), 0.25); // Componente ambiental mínima 0.25

    FragColor = vec4(baseColor.rgb * diff, baseColor.a);
}
