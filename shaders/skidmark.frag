#version 330 core
in vec4 vColor;
in vec2 vUV;

out vec4 FragColor;

void main() {
    // 1. Suavizado en los bordes laterales del neumatico (antialiasing procedural)
    // vUV.x va de 0.0 (borde izquierdo) a 1.0 (borde derecho)
    float edgeDist = abs(vUV.x - 0.5) * 2.0; // 0.0 en el centro, 1.0 en los extremos
    float edgeAlpha = smoothstep(1.0, 0.62, edgeDist);

    // 2. Ranuras y estriado caracteristico de banda de rodadura de competicion
    float tread = 0.86 + 0.14 * cos(vUV.x * 25.1327); // 4 bandas longitudinales

    float alpha = vColor.a * edgeAlpha * tread;
    FragColor = vec4(vColor.rgb, alpha);
}
