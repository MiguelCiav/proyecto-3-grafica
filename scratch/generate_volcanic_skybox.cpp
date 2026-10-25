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

struct Ember {
    Vec3 dir;
    float brightness;
    float size;
    Vec3 color;
};

bool saveTGA(const std::string& filename, int width, int height, const std::vector<uint8_t>& data) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;

    uint8_t header[18] = {0};
    header[2] = 2; // Uncompressed true-color image
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = 24; // 24 bits per pixel (BGR)
    header[17] = 0x20; // Top-left origin

    out.write(reinterpret_cast<char*>(header), 18);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

int main() {
    const int SIZE = 512;
    const std::string outDir = "assets/textures/skybox_volcanic";
    fs::create_directories(outDir);

    const std::string faceNames[6] = {
        "right.tga", "left.tga", "top.tga", "bottom.tga", "front.tga", "back.tga"
    };

    // 1. Generar 3.000 chispas y cenizas incandescentes (volcanic embers)
    std::mt19937 rng(8888);
    std::uniform_real_distribution<float> uDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> bDist(0.6f, 1.4f);
    std::uniform_real_distribution<float> cDist(0.0f, 1.0f);

    std::vector<Ember> embers;
    embers.reserve(3000);
    for (int i = 0; i < 3000; ++i) {
        Vec3 d(uDist(rng), uDist(rng), uDist(rng));
        while (d.length() < 0.05f) {
            d = Vec3(uDist(rng), uDist(rng), uDist(rng));
        }
        Ember e;
        e.dir = d.normalize();
        e.brightness = bDist(rng);
        e.size = (i < 80) ? 0.008f : ((i < 500) ? 0.005f : 0.003f);

        float c = cDist(rng);
        if (c < 0.45f)      e.color = Vec3(1.00f, 0.42f, 0.05f); // Naranja fuego vivo
        else if (c < 0.75f) e.color = Vec3(1.00f, 0.85f, 0.20f); // Oro incandescente de magma
        else if (c < 0.90f) e.color = Vec3(1.00f, 0.15f, 0.08f); // Rojo lava brillante
        else                e.color = Vec3(1.00f, 0.95f, 0.65f); // Blanco chispa ardiente

        embers.push_back(e);
    }

    std::cout << "[Skybox Volcanic Generator] Generando cubemap volcanico de caldera (512x512)..." << std::endl;

    // Paleta volcánica:
    const Vec3 colMagmaGlow(0.85f, 0.25f, 0.02f);   // Resplandor de magma en el horizonte
    const Vec3 colFierySky(0.35f, 0.08f, 0.04f);    // Cielo intermedio rojizo ardiente
    const Vec3 colSmokeDark(0.06f, 0.03f, 0.04f);   // Nubes de humo y ceniza densas
    const Vec3 colZenithSmoke(0.02f, 0.01f, 0.02f); // Cenit oscuro de ceniza volcánica

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

                // 1. Degradado vertical: Magma inferior -> Fuego intermedio -> Humo superior
                Vec3 color;
                if (rayDir.y < 0.1f) {
                    float t = (rayDir.y + 1.0f) / 1.1f;
                    color = lerp(Vec3(0.05f, 0.02f, 0.02f), colMagmaGlow, std::pow(t, 1.5f));
                } else if (rayDir.y < 0.6f) {
                    float t = (rayDir.y - 0.1f) / 0.5f;
                    color = lerp(colMagmaGlow, colFierySky, t);
                } else {
                    float t = (rayDir.y - 0.6f) / 0.4f;
                    color = lerp(colFierySky, colZenithSmoke, t);
                }

                // 2. Nubes turbulentas de humo y ceniza volcánica (armónicos multifrecuencia)
                float s1 = std::sin(rayDir.x * 4.0f + rayDir.y * 3.0f + rayDir.z * 3.5f);
                float s2 = std::cos(rayDir.x * 7.5f - rayDir.z * 6.0f + rayDir.y * 5.0f);
                float s3 = std::sin(rayDir.y * 11.0f + rayDir.x * 9.0f);
                float smokeDensity = clamp(s1 * 0.5f + s2 * 0.35f + s3 * 0.15f, -1.0f, 1.0f);

                if (smokeDensity > 0.1f) {
                    float ashBlend = (smokeDensity - 0.1f) * 0.7f;
                    color = lerp(color, colSmokeDark, ashBlend);
                }

                // 3. Resplandor de erupción volcánica en el horizonte norte (+Z)
                Vec3 eruptionCenter = Vec3(0.10f, 0.05f, 0.95f).normalize();
                float eDot = std::max(0.0f, rayDir.dot(eruptionCenter));
                if (eDot > 0.75f) {
                    float glow = std::pow(eDot, 5.0f);
                    Vec3 colPlume = Vec3(1.20f, 0.50f, 0.10f);
                    color = color + colPlume * (glow * 0.85f);
                }

                // 4. Siluetas de cordilleras volcánicas y cráteres dentados en el horizonte
                float volcanoRidge = std::sin(rayDir.x * 6.0f + rayDir.z * 5.0f) * 0.08f
                                   + std::cos(rayDir.x * 12.0f - rayDir.z * 10.0f) * 0.04f;
                if (rayDir.y < volcanoRidge && rayDir.y > -0.4f) {
                    float ridgeBlend = clamp((volcanoRidge - rayDir.y) / 0.25f, 0.0f, 0.90f);
                    Vec3 colBasaltRidge(0.04f, 0.02f, 0.02f);
                    color = lerp(color, colBasaltRidge, ridgeBlend);
                }

                // 5. Chispas y cenizas incandescentes (volcanic embers)
                for (const auto& e : embers) {
                    float dotVal = rayDir.dot(e.dir);
                    if (dotVal > 0.9991f) {
                        float dist = std::acos(clamp(dotVal, -1.0f, 1.0f));
                        if (dist < e.size) {
                            float emberGlow = 1.0f - (dist / e.size);
                            color = color + e.color * (emberGlow * e.brightness * 1.5f);
                        }
                    }
                }

                // Convertir a 8-bit BGR para formato TGA
                int idx = (y * SIZE + x) * 3;
                uint8_t r = static_cast<uint8_t>(clamp(color.x, 0.0f, 1.0f) * 255.0f);
                uint8_t g = static_cast<uint8_t>(clamp(color.y, 0.0f, 1.0f) * 255.0f);
                uint8_t b = static_cast<uint8_t>(clamp(color.z, 0.0f, 1.0f) * 255.0f);

                pixels[idx + 0] = b;
                pixels[idx + 1] = g;
                pixels[idx + 2] = r;
            }
        }

        std::string filePath = outDir + "/" + faceNames[face];
        if (!saveTGA(filePath, SIZE, SIZE, pixels)) {
            std::cerr << "Error al guardar: " << filePath << std::endl;
            return 1;
        }
        std::cout << "  [OK] Cara volcanica generada: " << faceNames[face] << std::endl;
    }

    std::cout << "[SUCCESS] Cubemap volcanico generado en: " << outDir << std::endl;
    return 0;
}
