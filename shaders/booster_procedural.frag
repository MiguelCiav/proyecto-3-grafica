#version 330 core

in vec2 v_TexCoords;
in vec3 v_FragPos;
in vec3 v_Normal;

out vec4 FragColor;

uniform float u_Time;
uniform float u_Speed;
uniform float u_Repetitions;
uniform float u_ArrowThickness;
uniform vec3 u_BaseColor;
uniform vec3 u_EnergyColor;
uniform vec3 u_CoreColor;
uniform float u_EmissionIntensity;
uniform float u_PulseSpeed;

void main()
{
    // Coordenadas UV normalizadas
    vec2 uv = v_TexCoords;

    // Valores por defecto seguros en caso de no suministrar uniforms
    float speed = (u_Speed != 0.0) ? u_Speed : 3.0;
    float reps = (u_Repetitions > 0.0) ? u_Repetitions : 4.0;
    float thickness = (u_ArrowThickness > 0.0) ? u_ArrowThickness : 0.20;
    float pulseSpeed = (u_PulseSpeed > 0.0) ? u_PulseSpeed : 5.0;
    float emission = (u_EmissionIntensity > 0.0) ? u_EmissionIntensity : 2.0;

    vec3 baseCol = (length(u_BaseColor) > 0.0) ? u_BaseColor : vec3(0.08, 0.08, 0.10);
    vec3 energyCol = (length(u_EnergyColor) > 0.0) ? u_EnergyColor : vec3(0.0, 0.9, 1.0);
    vec3 coreCol = (length(u_CoreColor) > 0.0) ? u_CoreColor : vec3(1.0, 1.0, 1.0);

    // 1. Desplazamiento dinámico continuo a lo largo del eje V (avance hacia adelante)
    float flow = uv.y * reps - u_Time * speed;
    float vAnim = fract(flow);

    // 2. Cálculo geométrico del patrón Chevron (Flecha)
    // arrowDist mide la distancia horizontal al centro escalada [0, 1]
    float arrowDist = abs(uv.x - 0.5) * 2.0;
    float slope = 0.45; // Pendiente de las alas de la flecha
    float chevronBase = arrowDist * slope;

    // Máscara de flecha suave con soporte anti-aliasing
    float delta = 0.02;
    float maskOuter = smoothstep(chevronBase - delta, chevronBase + delta, vAnim);
    float maskInner = 1.0 - smoothstep(chevronBase + thickness - delta, chevronBase + thickness + delta, vAnim);
    float arrowMask = clamp(maskOuter * maskInner, 0.0, 1.0);

    // Núcleo brillante centrado dentro del grosor de la flecha
    float arrowCenter = chevronBase + (thickness * 0.5);
    float coreDist = abs(vAnim - arrowCenter);
    float coreMask = 1.0 - smoothstep(0.0, thickness * 0.35, coreDist);
    coreMask *= arrowMask;

    // 3. Modulación de pulso y emisión sinusoidal
    float pulse = 0.82 + 0.18 * sin(u_Time * pulseSpeed);
    float energyPulse = pulse * emission;

    // Bandas luminosas en los bordes laterales del pad (guías de energía)
    float sideDist = min(uv.x, 1.0 - uv.x);
    float borderGlow = 1.0 - smoothstep(0.01, 0.08, sideDist);
    borderGlow *= (0.7 + 0.3 * sin(u_Time * pulseSpeed * 0.8 + uv.y * 10.0));

    // 4. Composición final del color mediante mezcla matemática
    vec3 finalColor = baseCol;

    // Resplandor difuso del asfalto/base cerca de las flechas
    float glowRadius = thickness * 1.6;
    float diffuseGlow = 1.0 - smoothstep(0.0, glowRadius, abs(vAnim - arrowCenter));
    finalColor += energyCol * diffuseGlow * 0.35 * pulse;

    // Aplicar color de energía en las flechas y bordes
    vec3 arrowEnergy = mix(energyCol, coreCol, coreMask * 0.85);
    finalColor = mix(finalColor, arrowEnergy * energyPulse, arrowMask);
    finalColor += energyCol * borderGlow * 1.2 * pulse;

    FragColor = vec4(finalColor, 1.0);
}
