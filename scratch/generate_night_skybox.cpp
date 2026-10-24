#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <filesystem>
#include <random>
#include <algorithm>

namespace fs = std::filesystem;

struct Vec3 {
    float x, y, z;
    Vec3(float x=0, float y=0, float z=0) : x(x), y(y), z(z) {}
    Vec3 operator+(const Vec3& o) const { return {x+o.x, y+o.y, z+o.z}; }
    Vec3 operator-(const Vec3& o) const { return {x-o.x, y-o.y, z-o.z}; }
    Vec3 operator*(float s) const { return {x*s, y*s, z*s}; }
    float dot(const Vec3& o) const { return x*o.x + y*o.y + z*o.z; }
    float length() const { return std::sqrt(dot(*this)); }
    Vec3 normalize() const {
        float l = length();
        return l > 1e-6f ? (*this) * (1.0f / l) : Vec3(0, 1, 0);
    }
};

struct Star {
    Vec3 dir;
    float brightness;
    float size;
    Vec3 color;
};

// Genera un archivo TGA 24-bit sin compresión
void writeTGA(const std::string& filename, int w, int h, const std::vector<uint8_t>& bgrData) {
    std::ofstream out(filename, std::ios::binary);
    if (!out) {
        std::cerr << "Error al abrir " << filename << std::endl;
        return;
    }
    uint8_t header[18] = {
        0, 0, 2, // Tipo 2: True-color no comprimido
        0, 0, 0, 0, 0,
        0, 0, 0, 0,
        (uint8_t)(w & 0xFF), (uint8_t)((w >> 8) & 0xFF),
        (uint8_t)(h & 0xFF), (uint8_t)((h >> 8) & 0xFF),
        24, // 24 bpp (BGR)
        0x20 // Top-left origin para coincidir con el mapeo de texturas
    };
    out.write(reinterpret_cast<char*>(header), 18);
    out.write(reinterpret_cast<const char*>(bgrData.data()), bgrData.size());
}

int main() {
    const int SIZE = 512;
    fs::create_directories("assets/textures/skybox_night");

    // 1. Generar estrellas sobre la esfera celeste con distribución uniforme
    std::mt19937 rng(42);
    std::uniform_real_distribution<float> uDist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> bDist(0.4f, 1.0f);
    std::uniform_real_distribution<float> colDist(0.0f, 1.0f);

    std::vector<Star> stars;
    stars.reserve(3000);
    for (int i = 0; i < 3000; ++i) {
        Vec3 d(uDist(rng), uDist(rng), uDist(rng));
        while (d.length() < 0.1f) {
            d = Vec3(uDist(rng), uDist(rng), uDist(rng));
        }
        Star s;
        s.dir = d.normalize();
        s.brightness = bDist(rng);
        if (i < 30) {
            s.brightness = 1.6f; // Estrellas alfa prominentes
            s.size = 0.007f;
        } else if (i < 200) {
            s.brightness = 1.2f;
            s.size = 0.005f;
        } else {
            s.size = 0.0035f;
        }

        // Variedad de color espectral (azulada, blanca, ambarina)
        float c = colDist(rng);
        if (c < 0.40f) s.color = Vec3(0.95f, 0.98f, 1.00f);       // Blanco puro
        else if (c < 0.75f) s.color = Vec3(0.75f, 0.88f, 1.00f);  // Blanco azulado
        else s.color = Vec3(1.00f, 0.88f, 0.65f);                 // Dorado calido

        stars.push_back(s);
    }

    // Dirección de la luna en el cielo nocturno
    Vec3 moonDir = Vec3(0.45f, 0.40f, 0.75f).normalize();

    // 6 caras del cubemap en orden OpenGL (+X, -X, +Y, -Y, +Z, -Z)
    const std::string faceNames[6] = { "right", "left", "top", "bottom", "front", "back" };

    std::cout << "Generando cubemap nocturno procedural (512x512 por cara)..." << std::endl;

    for (int face = 0; face < 6; ++face) {
        std::vector<uint8_t> bgr(SIZE * SIZE * 3);

        for (int y = 0; y < SIZE; ++y) {
            float v = (2.0f * (y + 0.5f) / SIZE) - 1.0f;
            for (int x = 0; x < SIZE; ++x) {
                float u = (2.0f * (x + 0.5f) / SIZE) - 1.0f;

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

                // 1. Gradiente atmosférico nocturno
                // Cenit (+Y) es azul marino muy oscuro; horizonte (Y=0) tiene una tenue bruma azulada
                float elevation = rayDir.y;
                Vec3 col;
                if (elevation >= 0.0f) {
                    float t = std::pow(1.0f - elevation, 1.8f);
                    Vec3 zenith(0.012f, 0.016f, 0.035f);    // Azul medianoche profundo
                    Vec3 horizon(0.035f, 0.055f, 0.110f);   // Bruma tenue nocturna en horizonte
                    col = zenith * (1.0f - t) + horizon * t;
                } else {
                    float t = std::min(1.0f, -elevation * 2.0f);
                    Vec3 horizon(0.035f, 0.055f, 0.110f);
                    Vec3 nadir(0.008f, 0.010f, 0.020f);     // Suelo oscuro
                    col = horizon * (1.0f - t) + nadir * t;
                }

                // 2. Luna brillante y halo lunar
                float moonDot = rayDir.dot(moonDir);
                if (moonDot > 0.9985f) {
                    // Disco lunar con borde suave
                    float moonCore = (moonDot - 0.9985f) / (1.0f - 0.9985f);
                    Vec3 moonCol(1.4f, 1.45f, 1.55f);
                    col = col + moonCol * std::pow(moonCore, 0.4f);
                } else if (moonDot > 0.95f) {
                    // Resplandor / halo lunar difuso
                    float halo = (moonDot - 0.95f) / (0.9985f - 0.95f);
                    Vec3 haloCol(0.12f, 0.16f, 0.28f);
                    col = col + haloCol * std::pow(halo, 3.0f);
                }

                // 3. Estrellas
                if (elevation > -0.05f) {
                    for (const auto& s : stars) {
                        float d = rayDir.dot(s.dir);
                        float threshold = 1.0f - (s.size * s.size * 0.5f);
                        if (d > threshold) {
                            float factor = (d - threshold) / (1.0f - threshold);
                            float intensity = std::pow(factor, 2.5f) * s.brightness;
                            col = col + s.color * intensity;
                        }
                    }
                }

                // Clamping y conversión a byte (BGR)
                int r = std::clamp<int>(static_cast<int>(col.x * 255.0f), 0, 255);
                int g = std::clamp<int>(static_cast<int>(col.y * 255.0f), 0, 255);
                int b = std::clamp<int>(static_cast<int>(col.z * 255.0f), 0, 255);

                int idx = (y * SIZE + x) * 3;
                bgr[idx + 0] = static_cast<uint8_t>(b);
                bgr[idx + 1] = static_cast<uint8_t>(g);
                bgr[idx + 2] = static_cast<uint8_t>(r);
            }
        }

        std::string filename = "assets/textures/skybox_night/" + faceNames[face] + ".tga";
        writeTGA(filename, SIZE, SIZE, bgr);
        std::cout << "  Cara guardada: " << filename << std::endl;
    }

    std::cout << "Skybox nocturno generado exitosamente con 3000 estrellas y luna!" << std::endl;
    return 0;
}
