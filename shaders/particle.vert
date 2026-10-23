#version 330 core

// Atributos por vértice del cuadrilátero base
layout (location = 0) in vec2 a_QuadPos;
layout (location = 1) in vec2 a_TexCoords;

// Atributos por instancia de partícula (VBO dinámico instanciado)
layout (location = 2) in vec3 a_ParticlePos;
layout (location = 3) in float a_ParticleSize;
layout (location = 4) in vec4 a_ParticleColor;

out vec2 v_TexCoords;
out vec4 v_Color;

uniform mat4 u_View;
uniform mat4 u_Projection;

void main()
{
    v_TexCoords = a_TexCoords;
    v_Color = a_ParticleColor;

    // Billboarding esférico: extracción de vectores Right y Up en espacio del mundo
    vec3 cameraRight = vec3(u_View[0][0], u_View[1][0], u_View[2][0]);
    vec3 cameraUp    = vec3(u_View[0][1], u_View[1][1], u_View[2][1]);

    // Posicionamiento del vértice en coordenadas de mundo alineado a la vista
    vec3 worldPos = a_ParticlePos 
                  + (cameraRight * a_QuadPos.x + cameraUp * a_QuadPos.y) * a_ParticleSize;

    gl_Position = u_Projection * u_View * vec4(worldPos, 1.0);
}
