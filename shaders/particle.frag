#version 330 core

in vec2 v_TexCoords;
in vec4 v_Color;

out vec4 FragColor;

uniform bool u_UseTexture;
uniform sampler2D u_Texture;

void main()
{
    // Cálculo de distancia radial desde el centro del quad [0, 1]
    vec2 centerOffset = v_TexCoords - vec2(0.5);
    float dist = length(centerOffset) * 2.0;
    if (dist > 1.0) {
        discard;
    }

    // Suavizado radial continuo para partículas difusas y sin cortes angulares
    float radialFalloff = smoothstep(1.0, 0.0, dist);

    vec4 texColor = vec4(1.0);
    if (u_UseTexture) {
        texColor = texture(u_Texture, v_TexCoords);
    }

    vec4 finalColor = v_Color * texColor;
    finalColor.a *= radialFalloff;

    if (finalColor.a < 0.005) {
        discard;
    }

    FragColor = finalColor;
}
