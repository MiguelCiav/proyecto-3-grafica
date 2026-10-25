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

struct Plankton {
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
    const std::string outDir = "assets/textures/skybox_aquatic";
    fs::create_directories(outDir);

    const std::string faceNames[6] = {
        "right.tga", "left.tga", "top.tga", "bottom.tga", "front.tga", "back.tga"
    };

    // 1. Generar partículas de plancton y esporas marinas bioluminiscentes
    std::mt19937 rng(4242);
    std::uniform_real_distribution<float> uDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> bDist(0.4f, 1.2f);
    std::uniform_real_distribution<float> cDist(0.0f, 1.0f);

    std::vector<Plankton> particles;
    particles.reserve(2500);
    for (int i = 0; i < 2500; ++i) {
        Vec3 d(uDist(rng), uDist(rng), uDist(rng));
        while (d.length() < 0.05f) {
            d = Vec3(uDist(rng), uDist(rng), uDist(rng));
        }
        Plankton p;
        p.dir = d.normalize();
        p.brightness = bDist(rng);
        p.size = (i < 50) ? 0.007f : ((i < 400) ? 0.0045f : 0.0028f);

        float c = cDist(rng);
        if (c < 0.45f)      p.color = Vec3(0.15f, 0.95f, 0.85f); // Turquesa bioluminiscente
        else if (c < 0.75f) p.color = Vec3(0.30f, 0.70f, 1.00f); // Azul zafiro acuático
        else if (c < 0.90f) p.color = Vec3(0.40f, 1.00f, 0.60f); // Verde esmeralda marino
        else                p.color = Vec3(0.95f, 0.50f, 0.90f); // Púrpura coralino

        particles.push_back(p);
    }

    std::cout << "[Skybox Aquatic Generator] Generando cubemap oceanico abisal procedural (512x512)..." << std::endl;

    // Gradiente submarino:
    // Fondo abisal profundo (abajo): azul marino casi negro
    const Vec3 colDeepAbyss(0.008f, 0.035f, 0.075f);
    // Aguas medias: azul océano profundo
    const Vec3 colMidOcean(0.020f, 0.120f, 0.220f);
    // Aguas superiores: cian turquesa vibrante
    const Vec3 colSurfaceWater(0.080f, 0.450f, 0.620f);
    // Resplandor del sol penetrando desde la superficie (arriba): blanco dorado/turquesa intenso
    const Vec3 colSunSurface(0.550f, 0.900f, 0.950f);

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

                // 1. Gradiente de profundidad oceánica basado en Y
                // rayDir.y va de -1.0 (fondo marino) a +1.0 (superficie del agua)
                Vec3 color;
                if (rayDir.y < 0.0f) {
                    float t = (rayDir.y + 1.0f); // 0 (fondo) a 1 (horizonte)
                    color = lerp(colDeepAbyss, colMidOcean, std::pow(t, 1.2f));
                } else {
                    float t = rayDir.y; // 0 (horizonte) a 1 (cenit / superficie)
                    color = lerp(colMidOcean, colSurfaceWater, t);
                }

                // 2. Rayos de luz crepusculares submarinos (God Rays cáusticos desde la superficie)
                if (rayDir.y > -0.3f) {
                    float surfaceT = (rayDir.y + 0.3f) / 1.3f;
                    // Proyección de ondas cáusticas
                    float caustic1 = std::sin(rayDir.x * 12.0f + rayDir.z * 10.0f) * 0.5f + 0.5f;
                    float caustic2 = std::cos(rayDir.x * 18.0f - rayDir.z * 15.0f) * 0.5f + 0.5f;
                    float causticCombined = std::pow(caustic1 * caustic2, 2.0f);

                    // Haz de luz solar principal cenital
                    Vec3 sunZenithDir = Vec3(0.15f, 0.95f, 0.20f).normalize();
                    float sunDot = std::max(0.0f, rayDir.dot(sunZenithDir));
                    float sunCone = std::pow(sunDot, 6.0f);

                    Vec3 godRay = colSunSurface * (causticCombined * 0.35f * surfaceT + sunCone * 0.65f * surfaceT);
                    color = color + godRay;
                }

                // 3. Montañas de relieve submarino y trincheras abisales en el horizonte (-0.15 < Y < 0.15)
                float horizElevation = std::sin(rayDir.x * 4.0f + rayDir.z * 3.5f) * 0.07f
                                     + std::cos(rayDir.z * 7.0f - rayDir.x * 5.0f) * 0.04f;
                if (rayDir.y < horizElevation && rayDir.y > -0.4f) {
                    float silhouette = (horizElevation - rayDir.y) / (horizElevation + 0.4f);
                    Vec3 ridgeColor(0.005f, 0.025f, 0.055f);
                    color = lerp(color, ridgeColor, clamp(silhouette * 0.65f, 0.0f, 0.75f));
                }

                // 4. Plancton bioluminiscente disperso flotando en el agua
                for (const auto& p : particles) {
                    float dotVal = rayDir.dot(p.dir);
                    if (dotVal > 0.9992f) {
                        float dist = std::acos(clamp(dotVal, -1.0f, 1.0f));
                        if (dist < p.size) {
                            float glow = 1.0f - (dist / p.size);
                            color = color + p.color * (glow * p.brightness * 1.3f);
                        }
                    }
                }

                // Convertir a 8-bit BGR para TGA
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
        std::cout << "  [OK] Cara generada: " << faceNames[face] << std::endl;
    }

    std::cout << "[SUCCESS] Cubemap oceanico abisal generado en: " << outDir << std::endl;
    return 0;
}
