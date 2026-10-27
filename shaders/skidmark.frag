#version 330 core
in vec4 vColor;
in vec2 vUV;
in float vSpawnTime;

uniform float u_CurrentTime;
uniform float u_FadeDuration;

out vec4 FragColor;

void main() {
    // 1. Desvanecimiento dinámico en GPU según tiempo transcurrido desde spawn
    float fadeDur = (u_FadeDuration > 0.0) ? u_FadeDuration : 4.0;
    float age = max(0.0, u_CurrentTime - vSpawnTime);
    float alphaFade = clamp(1.0 - (age / fadeDur), 0.0, 1.0);

    if (alphaFade <= 0.0) {
        discard;
    }

    // 2. Suavizado en los bordes laterales del neumatico (antialiasing procedural)
    // vUV.x va de 0.0 (borde izquierdo) a 1.0 (borde derecho)
    float edgeDist = abs(vUV.x - 0.5) * 2.0; // 0.0 en el centro, 1.0 en los extremos
    float edgeAlpha = smoothstep(1.0, 0.62, edgeDist);

    // 3. Ranuras y estriado caracteristico de banda de rodadura de competicion
    float tread = 0.86 + 0.14 * cos(vUV.x * 25.1327); // 4 bandas longitudinales

    float alpha = vColor.a * alphaFade * edgeAlpha * tread;
    if (alpha <= 0.002) {
        discard;
    }

    FragColor = vec4(vColor.rgb, alpha);
}
