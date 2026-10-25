#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
#include <random>
#include <filesystem>
#include <cstdint>

namespace fs = std::filesystem;

struct Vec3 {
    float x, y, z;
    Vec3(float x_ = 0, float y_ = 0, float z_ = 0) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    float length() const { return std::sqrt(x * x + y * y + z * z); }
    Vec3 normalize() const {
        float len = length();
        if (len < 1e-6f) return Vec3(0, 1, 0);
        return Vec3(x / len, y / len, z / len);
    }
};

static inline Vec3 lerp(const Vec3& a, const Vec3& b, float t) {
    t = std::max(0.0f, std::min(1.0f, t));
    return a + (b - a) * t;
}

static inline float clamp(float v, float mn, float mx) {
    return std::max(mn, std::min(mx, v));
}

struct Star {
    Vec3 dir;
    float brightness;
    float size;
    Vec3 color;
};

bool saveTGA(const std::string& filename, int width, int height, const std::vector<uint8_t>& data) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;

    uint8_t header[18] = {0};
    header[2] = 2; // RGB sin compresión
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = 24; // 24 bits por pixel (BGR)
    header[17] = 0x20; // Origen superior-izquierdo (top-down)

    out.write(reinterpret_cast<char*>(header), 18);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

int main() {
    const int SIZE = 512;
    const std::string outDir = "assets/textures/skybox_cosmic";
    fs::create_directories(outDir);

    const std::string faceNames[6] = {
        "right.tga", "left.tga", "top.tga", "bottom.tga", "front.tga", "back.tga"
    };

    // 1. Generar 4.000 estrellas cósmicas uniformemente sobre la esfera celeste
    std::mt19937 rng(1337);
    std::uniform_real_distribution<float> uDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> bDist(0.5f, 1.0f);
    std::uniform_real_distribution<float> cDist(0.0f, 1.0f);

    std::vector<Star> stars;
    stars.reserve(4500);
    for (int i = 0; i < 4500; ++i) {
        Vec3 d(uDist(rng), uDist(rng), uDist(rng));
        while (d.length() < 0.05f) {
            d = Vec3(uDist(rng), uDist(rng), uDist(rng));
        }
        Star s;
        s.dir = d.normalize();
        s.brightness = bDist(rng);
        if (i < 40) {
            s.brightness = 1.9f;
            s.size = 0.0075f;
        } else if (i < 300) {
            s.brightness = 1.35f;
            s.size = 0.0055f;
        } else {
            s.size = 0.0035f;
        }

        float c = cDist(rng);
        if (c < 0.35f)      s.color = Vec3(0.92f, 0.96f, 1.00f); // Blanco estelar puro
        else if (c < 0.65f) s.color = Vec3(0.65f, 0.85f, 1.00f); // Azul brillante de estrella B/O
        else if (c < 0.85f) s.color = Vec3(1.00f, 0.88f, 0.65f); // Dorado cálido de estrella G/K
        else                s.color = Vec3(1.00f, 0.55f, 0.75f); // Rosa violeta cósmico

        stars.push_back(s);
    }

    // 2. Parámetros de los cuerpos celestes:
    // Planeta gigante anillado (Gas Giant con anillos en el horizonte frontal-derecho)
    const Vec3 planetCenter = Vec3(0.58f, 0.32f, 0.75f).normalize();
    const float planetRadius = 0.22f; // Radio angular en la esfera celeste
    const Vec3 ringNormal = Vec3(0.35f, 0.90f, 0.25f).normalize(); // Inclinación axial de los anillos

    // Planeta terrestre lejano (Azul turquesa con continentes en dirección opuesta)
    const Vec3 earthCenter = Vec3(-0.65f, 0.45f, -0.60f).normalize();
    const float earthRadius = 0.09f;

    std::cout << "[Skybox Cosmic Generator] Generando cubemap cósmico con nebulosas y planetas (512x512)..." << std::endl;

    for (int face = 0; face < 6; ++face) {
        std::vector<uint8_t> pixels(SIZE * SIZE * 3);

        for (int y = 0; y < SIZE; ++y) {
            float v = (2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(SIZE)) - 1.0f;

            for (int x = 0; x < SIZE; ++x) {
                float u = (2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(SIZE)) - 1.0f;

                Vec3 rayDir;
                switch (face) {
                    case 0: rayDir = Vec3( 1.0f,   -v,   -u); break; // +X (right)
                    case 1: rayDir = Vec3(-1.0f,   -v,    u); break; // -X (left)
                    case 2: rayDir = Vec3(    u, 1.0f,    v); break; // +Y (top)
                    case 3: rayDir = Vec3(    u,-1.0f,   -v); break; // -Y (bottom)
                    case 4: rayDir = Vec3(    u,   -v,  1.0f); break; // +Z (front)
                    case 5: rayDir = Vec3(   -u,   -v, -1.0f); break; // -Z (back)
                }
                rayDir = rayDir.normalize();

                // 1. Fondo del espacio interestelar profundo
                Vec3 color(0.012f, 0.010f, 0.024f);

                // 2. Nebulosas galácticas volumétricas procedurales (Armónicos multifrecuencia)
                float n1 = std::sin(rayDir.x * 2.5f + rayDir.y * 3.0f + rayDir.z * 2.0f);
                float n2 = std::cos(rayDir.x * 5.0f - rayDir.z * 4.5f + rayDir.y * 3.5f);
                float n3 = std::sin(rayDir.y * 7.0f + rayDir.x * 6.0f);
                float nebulaDensity = std::max(0.0f, (n1 * 0.55f + n2 * 0.35f + n3 * 0.20f));

                // Brazo de nebulosa magenta/púrpura
                float magentaArm = std::pow(std::max(0.0f, std::sin(rayDir.x * 2.0f + rayDir.z * 3.0f) * 0.5f + 0.5f), 3.0f);
                Vec3 colMagenta(0.68f, 0.12f, 0.58f);

                // Brazo de nebulosa cian/turquesa
                float cyanArm = std::pow(std::max(0.0f, std::cos(rayDir.y * 2.5f - rayDir.x * 3.5f) * 0.5f + 0.5f), 3.5f);
                Vec3 colCyan(0.05f, 0.55f, 0.78f);

                // Brillo de gas estelar en el espacio
                color = color + (colMagenta * magentaArm * nebulaDensity * 0.45f)
                              + (colCyan * cyanArm * nebulaDensity * 0.40f);

                // 3. Estrellas con brillo y difracción estelar
                for (const auto& s : stars) {
                    float dotVal = rayDir.dot(s.dir);
                    if (dotVal > 0.9990f) {
                        float dist = std::acos(clamp(dotVal, -1.0f, 1.0f));
                        if (dist < s.size) {
                            float starGlow = 1.0f - (dist / s.size);
                            color = color + s.color * (starGlow * s.brightness);
                        }
                    }
                }

                // 4. Planeta Gigante Anillado (Estilo Saturno cósmico)
                float pDot = rayDir.dot(planetCenter);
                if (pDot > 0.85f) {
                    float angleToPlanet = std::acos(clamp(pDot, -1.0f, 1.0f));

                    // Anillos de polvo de hielo concéntricos
                    float ringPlaneDist = std::abs(rayDir.dot(ringNormal));
                    float distFromCenter = angleToPlanet;
                    if (ringPlaneDist < 0.025f && distFromCenter > planetRadius * 1.25f && distFromCenter < planetRadius * 2.65f) {
                        float ringBands = std::sin(distFromCenter * 80.0f) * 0.5f + 0.5f;
                        float ringAlpha = (1.0f - ringPlaneDist / 0.025f) * (0.65f + 0.35f * ringBands);
                        Vec3 ringColor(0.85f, 0.75f, 0.55f);
                        color = lerp(color, ringColor, ringAlpha * 0.85f);
                    }

                    // Esfera del planeta gaseoso
                    if (angleToPlanet < planetRadius) {
                        float relDist = angleToPlanet / planetRadius;
                        // Bandas atmosféricas
                        float band = std::sin(rayDir.y * 45.0f) * 0.15f;
                        Vec3 basePlanet = Vec3(0.88f, 0.65f, 0.42f) + Vec3(band, band * 0.7f, 0.0f);

                        // Resplandor de borde atmosférico (Fresnel limb)
                        float limb = std::pow(relDist, 2.5f);
                        Vec3 limbColor(0.40f, 0.75f, 1.00f);
                        Vec3 planetColor = lerp(basePlanet, limbColor, limb * 0.70f);

                        color = planetColor;
                    }
                }

                // 5. Exoplaneta azul lejano
                float eDot = rayDir.dot(earthCenter);
                if (eDot > 0.985f) {
                    float angle = std::acos(clamp(eDot, -1.0f, 1.0f));
                    if (angle < earthRadius) {
                        float t = angle / earthRadius;
                        // Océano azul con nubes blancas
                        float clouds = std::sin(rayDir.x * 35.0f + rayDir.y * 25.0f);
                        Vec3 ocean(0.12f, 0.40f, 0.85f);
                        Vec3 cloud(0.95f, 0.98f, 1.00f);
                        Vec3 eColor = (clouds > 0.3f) ? cloud : ocean;
                        // Resplandor atmosférico de atmósfera azul
                        eColor = lerp(eColor, Vec3(0.35f, 0.75f, 1.00f), std::pow(t, 2.0f) * 0.65f);
                        color = eColor;
                    }
                }

                // Clamp final a 8-bit BGR
                uint8_t r = static_cast<uint8_t>(clamp(color.x, 0.0f, 1.0f) * 255.0f);
                uint8_t g = static_cast<uint8_t>(clamp(color.y, 0.0f, 1.0f) * 255.0f);
                uint8_t b = static_cast<uint8_t>(clamp(color.z, 0.0f, 1.0f) * 255.0f);

                int idx = (y * SIZE + x) * 3;
                pixels[idx + 0] = b;
                pixels[idx + 1] = g;
                pixels[idx + 2] = r;
            }
        }

        std::string path = outDir + "/" + faceNames[face];
        if (saveTGA(path, SIZE, SIZE, pixels)) {
            std::cout << "  [OK] Cara cósmica generada: " << faceNames[face] << std::endl;
        } else {
            std::cerr << "  [ERROR] No se pudo guardar: " << path << std::endl;
        }
    }

    std::cout << "[Skybox Cosmic Generator] Completado con exito!" << std::endl;
    return 0;
}
