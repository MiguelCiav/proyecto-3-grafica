#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <string>
#include <algorithm>
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

// Guardar imagen TGA 24-bit sin compresión con origen superior-izquierdo
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
    const std::string outDir = "assets/textures/skybox_desert";
    fs::create_directories(outDir);

    const std::string faceNames[6] = {
        "right.tga", "left.tga", "top.tga", "bottom.tga", "front.tga", "back.tga"
    };

    // Paleta de colores atmosféricos del desierto
    const Vec3 colZenith(0.18f, 0.52f, 0.92f);     // Azul cielo radiante en el cenit
    const Vec3 colMidSky(0.45f, 0.74f, 0.96f);     // Celeste cálido y abierto
    const Vec3 colHaze(0.96f, 0.82f, 0.60f);       // Bruma solar / polvo dorado en suspensión
    const Vec3 colDuneLit(0.92f, 0.74f, 0.48f);    // Dunas iluminadas por el sol
    const Vec3 colDuneShadow(0.80f, 0.56f, 0.36f); // Dunas en semisombra
    const Vec3 colGround(0.89f, 0.74f, 0.48f);     // Suelo de arena dorada (idéntico al plano de pista)

    // Posición del sol abrasador (alto en el cielo sur-este)
    const Vec3 sunDir = Vec3(0.35f, 0.68f, -0.65f).normalize();

    std::cout << "[Skybox Desert Generator] Generando cubemap corregido del desierto (512x512)..." << std::endl;

    for (int face = 0; face < 6; ++face) {
        std::vector<uint8_t> pixels(SIZE * SIZE * 3);

        for (int y = 0; y < SIZE; ++y) {
            // y=0 es la parte SUPERIOR de la imagen (v=-1, por tanto -v = +1 -> ARRIBA/CIELO)
            // y=SIZE-1 es la parte INFERIOR de la imagen (v=+1, por tanto -v = -1 -> ABAJO/SUELO)
            float v = (2.0f * (static_cast<float>(y) + 0.5f) / static_cast<float>(SIZE)) - 1.0f;

            for (int x = 0; x < SIZE; ++x) {
                float u = (2.0f * (static_cast<float>(x) + 0.5f) / static_cast<float>(SIZE)) - 1.0f;

                Vec3 rayDir;
                switch (face) {
                    case 0: rayDir = Vec3( 1.0f,   -v,   -u); break; // +X (right)
                    case 1: rayDir = Vec3(-1.0f,   -v,    u); break; // -X (left)
                    case 2: rayDir = Vec3(    u, 1.0f,    v); break; // +Y (top: puro cielo cenital)
                    case 3: rayDir = Vec3(    u,-1.0f,   -v); break; // -Y (bottom: pura arena)
                    case 4: rayDir = Vec3(    u,   -v,  1.0f); break; // +Z (front)
                    case 5: rayDir = Vec3(   -u,   -v, -1.0f); break; // -Z (back)
                }
                rayDir = rayDir.normalize();

                Vec3 color;

                // Caso especial: Cara inferior (-Y)
                if (face == 3) {
                    // Arena dorada cálida con sutil variación
                    float noise = 0.02f * std::sin(u * 15.0f) * std::cos(v * 15.0f);
                    color = colGround * (1.0f + noise);
                }
                // Caso especial: Cara superior (+Y)
                else if (face == 2) {
                    // Gradiente cenital celeste vibrante
                    float distFromCenter = std::sqrt(u * u + v * v);
                    color = lerp(colZenith, colMidSky, clamp(distFromCenter * 0.7f, 0.0f, 1.0f));
                }
                // Caras laterales (+X, -X, +Z, -Z): Bóveda celeste y horizonte de dunas
                else {
                    float elevation = rayDir.y;
                    float azimuth = std::atan2(rayDir.z, rayDir.x);

                    // Silueta suave de dunas distantes en el horizonte
                    float duneProfile = 0.025f * std::sin(3.0f * azimuth + 0.5f)
                                      + 0.015f * std::cos(7.0f * azimuth - 1.0f)
                                      + 0.008f * std::sin(13.0f * azimuth);
                    float duneHorizon = 0.01f + duneProfile;

                    if (elevation > duneHorizon) {
                        // Cielo sobre el horizonte
                        float skyT = clamp((elevation - duneHorizon) / 0.55f, 0.0f, 1.0f);
                        if (skyT < 0.25f) {
                            // Bruma dorada cerca del horizonte
                            float tHaze = skyT / 0.25f;
                            color = lerp(colHaze, colMidSky, tHaze);
                        } else {
                            // Transición hacia el azul brillante del cenit
                            float tZenith = (skyT - 0.25f) / 0.75f;
                            color = lerp(colMidSky, colZenith, tZenith);
                        }

                        // Capas tenues de nubes cirros en el cielo
                        float cloudNoise = std::sin(rayDir.x * 6.0f + rayDir.z * 4.0f) * std::cos(rayDir.y * 12.0f);
                        if (elevation > 0.20f && elevation < 0.60f && cloudNoise > 0.45f) {
                            float cloudAlpha = (cloudNoise - 0.45f) * 0.40f;
                            Vec3 cloudCol(1.0f, 0.98f, 0.94f);
                            color = lerp(color, cloudCol, cloudAlpha);
                        }
                    } else {
                        // Dunas y suelo debajo del horizonte
                        float depth = (duneHorizon - elevation) / 0.15f;
                        depth = clamp(depth, 0.0f, 1.0f);
                        // Sombreado de dunas según orientación solar
                        float sunFacing = std::max(0.0f, rayDir.dot(sunDir));
                        Vec3 duneCol = lerp(colDuneShadow, colDuneLit, sunFacing);
                        color = lerp(duneCol, colGround, depth);
                    }
                }

                // Resplandor del Sol abrasador (si está en el campo de visión de esta cara)
                float sunDot = rayDir.dot(sunDir);
                if (sunDot > 0.0f) {
                    if (sunDot > 0.9985f) {
                        // Disco solar blanco puro deslumbrante
                        color = Vec3(1.0f, 1.0f, 0.96f);
                    } else {
                        float angle = std::acos(clamp(sunDot, -1.0f, 1.0f));
                        // Corona solar dorada
                        float corona = std::exp(-std::pow(angle / 0.10f, 2.0f)) * 1.5f;
                        float flare  = std::exp(-std::pow(angle / 0.32f, 2.0f)) * 0.5f;
                        Vec3 sunGlow(1.0f, 0.92f, 0.65f);
                        color = color + sunGlow * (corona + flare);
                    }
                }

                // Clamp final a [0, 1] y empaquetado en BGR para formato TGA
                uint8_t r = static_cast<uint8_t>(clamp(color.x, 0.0f, 1.0f) * 255.0f);
                uint8_t g = static_cast<uint8_t>(clamp(color.y, 0.0f, 1.0f) * 255.0f);
                uint8_t b = static_cast<uint8_t>(clamp(color.z, 0.0f, 1.0f) * 255.0f);

                int idx = (y * SIZE + x) * 3;
                pixels[idx + 0] = b; // B
                pixels[idx + 1] = g; // G
                pixels[idx + 2] = r; // R
            }
        }

        std::string path = outDir + "/" + faceNames[face];
        if (saveTGA(path, SIZE, SIZE, pixels)) {
            std::cout << "  [OK] Cara generada correctamente: " << faceNames[face] << std::endl;
        } else {
            std::cerr << "  [ERROR] No se pudo guardar: " << path << std::endl;
        }
    }

    std::cout << "[Skybox Desert Generator] Completado con exito!" << std::endl;
    return 0;
}
