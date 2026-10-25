#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
#include <filesystem>

namespace fs = std::filesystem;

struct Vec3 {
    float x, y, z;
    Vec3(float x_ = 0, float y_ = 0, float z_ = 0) : x(x_), y(y_), z(z_) {}
    Vec3 operator+(const Vec3& o) const { return Vec3(x + o.x, y + o.y, z + o.z); }
    Vec3 operator-(const Vec3& o) const { return Vec3(x - o.x, y - o.y, z - o.z); }
    Vec3 operator*(float s) const { return Vec3(x * s, y * s, z * s); }
    float dot(const Vec3& o) const { return x * o.x + y * o.y + z * o.z; }
    Vec3 normalized() const {
        float len = std::sqrt(x * x + y * y + z * z);
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

// Direccion en esfera a partir de cara de cubemap y coordenadas (u, v) en [-1, 1]
Vec3 getCubeDirection(int face, float u, float v) {
    switch (face) {
        case 0: return Vec3( 1.0f,   -v,   -u).normalized(); // +X: right
        case 1: return Vec3(-1.0f,   -v,    u).normalized(); // -X: left
        case 2: return Vec3(   u,  1.0f,    v).normalized(); // +Y: top
        case 3: return Vec3(   u, -1.0f,   -v).normalized(); // -Y: bottom
        case 4: return Vec3(   u,   -v,  1.0f).normalized(); // +Z: front
        case 5: return Vec3(  -u,   -v, -1.0f).normalized(); // -Z: back
    }
    return Vec3(0, 1, 0);
}

// Guardar imagen TGA 24-bit sin compresion
bool saveTGA(const std::string& filename, int width, int height, const std::vector<unsigned char>& data) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) return false;

    unsigned char header[18] = {0};
    header[2] = 2; // RGB sin compresion
    header[12] = width & 0xFF;
    header[13] = (width >> 8) & 0xFF;
    header[14] = height & 0xFF;
    header[15] = (height >> 8) & 0xFF;
    header[16] = 24; // 24 bits por pixel
    header[17] = 0x20; // Origen superior izquierdo (top-down)

    out.write(reinterpret_cast<char*>(header), 18);
    out.write(reinterpret_cast<const char*>(data.data()), data.size());
    return true;
}

int main() {
    const int SIZE = 512;
    const std::string outDir = "assets/textures/skybox_desert";
    fs::create_directories(outDir);

    const std::string faceNames[6] = {
        "right.tga", "left.tga", "top.tga", "bottom.tga", "front.tga", "back.tga"
    };

    // Paleta de colores atmosfericos del desierto
    const Vec3 colZenith(0.18f, 0.52f, 0.88f);     // Azul cielo brillante
    const Vec3 colMidSky(0.42f, 0.72f, 0.94f);     // Celeste calido
    const Vec3 colHaze(0.94f, 0.76f, 0.48f);       // Calina / polvo de arena ambar
    const Vec3 colDune(0.86f, 0.62f, 0.38f);       // Dunas de arena dorada
    const Vec3 colMesa(0.68f, 0.36f, 0.22f);       // Canones de arenisca rojiza
    const Vec3 colGround(0.80f, 0.58f, 0.36f);     // Suelo desertico profundo

    // Posicion del sol abrasador (en alto hacia el sur-este)
    const Vec3 sunDir = Vec3(0.40f, 0.72f, -0.56f).normalized();

    std::cout << "[Skybox Desert Generator] Generando cubemap procedural del desierto (512x512)..." << std::endl;

    for (int face = 0; face < 6; ++face) {
        std::vector<unsigned char> pixels(SIZE * SIZE * 3);

        for (int y = 0; y < SIZE; ++y) {
            float v = 1.0f - 2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(SIZE);

            for (int x = 0; x < SIZE; ++x) {
                float u = 2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(SIZE) - 1.0f;

                Vec3 dir = getCubeDirection(face, u, v);

                // Angulo azimutal continuo sobre el horizonte
                float azimuth = std::atan2(dir.z, dir.x);

                // Siluetas procedurales de mesetas y dunas en el horizonte
                float mesaProfile = 0.035f * std::sin(3.0f * azimuth + 0.8f)
                                  + 0.025f * std::cos(7.0f * azimuth - 1.2f)
                                  + 0.015f * std::sin(13.0f * azimuth + 2.1f)
                                  + 0.008f * std::cos(23.0f * azimuth);
                float mesaHeight = 0.02f + std::max(0.0f, mesaProfile);

                // Dunas mas cercanas
                float duneProfile = 0.020f * std::sin(5.0f * azimuth - 0.5f)
                                  + 0.012f * std::cos(11.0f * azimuth + 1.4f);
                float duneHeight = -0.01f + duneProfile;

                Vec3 color;

                if (dir.y > 0.0f) {
                    // Gradiente del cielo
                    if (dir.y > 0.40f) {
                        float t = (dir.y - 0.40f) / 0.60f;
                        color = lerp(colMidSky, colZenith, t);
                    } else {
                        float t = dir.y / 0.40f;
                        color = lerp(colHaze, colMidSky, t);
                    }

                    // Si esta cerca del horizonte, mezclar con las mesetas distantes
                    if (dir.y < mesaHeight) {
                        float mFactor = (mesaHeight - dir.y) / 0.03f;
                        mFactor = clamp(mFactor, 0.0f, 1.0f);
                        Vec3 hazyMesa = lerp(colHaze, colMesa, 0.70f);
                        color = lerp(color, hazyMesa, mFactor);
                    }
                } else {
                    // Debajo del horizonte: Dunas y suelo desertico
                    float depth = -dir.y;
                    if (dir.y > duneHeight) {
                        color = lerp(colHaze, colDune, clamp((-dir.y) / 0.05f, 0.0f, 1.0f));
                    } else {
                        float t = clamp(depth * 3.0f, 0.0f, 1.0f);
                        color = lerp(colDune, colGround, t);
                    }
                }

                // Sol abrasador brillante y corona solar
                float sunDot = dir.dot(sunDir);
                if (sunDot > 0.0f) {
                    // Disco solar definido
                    if (sunDot > 0.9985f) {
                        color = Vec3(1.0f, 1.0f, 0.95f);
                    } else {
                        // Resplandor / corona dorada del sol
                        float angle = std::acos(clamp(sunDot, -1.0f, 1.0f));
                        float corona = std::exp(-std::pow(angle / 0.12f, 2.0f));
                        float outerGlow = std::exp(-std::pow(angle / 0.35f, 2.0f)) * 0.45f;
                        Vec3 sunColor = Vec3(1.0f, 0.90f, 0.55f);
                        color = color + sunColor * (corona * 1.4f + outerGlow);
                    }
                }

                // Calina de calor suave sobre el horizonte (|y| < 0.08)
                float horizonHaze = std::exp(-std::pow(dir.y / 0.06f, 2.0f)) * 0.25f;
                color = color + colHaze * horizonHaze;

                // Clamp y conversion a 8-bit BGR para TGA
                unsigned char r = static_cast<unsigned char>(clamp(color.x, 0.0f, 1.0f) * 255.0f);
                unsigned char g = static_cast<unsigned char>(clamp(color.y, 0.0f, 1.0f) * 255.0f);
                unsigned char b = static_cast<unsigned char>(clamp(color.z, 0.0f, 1.0f) * 255.0f);

                int idx = (y * SIZE + x) * 3;
                pixels[idx + 0] = b; // TGA es BGR
                pixels[idx + 1] = g;
                pixels[idx + 2] = r;
            }
        }

        std::string path = outDir + "/" + faceNames[face];
        if (saveTGA(path, SIZE, SIZE, pixels)) {
            std::cout << "  [OK] Cara generada: " << faceNames[face] << std::endl;
        } else {
            std::cerr << "  [ERROR] No se pudo guardar: " << path << std::endl;
        }
    }

    std::cout << "[Skybox Desert Generator] Completado con exito!" << std::endl;
    return 0;
}
