#include "world/PropSpawner.hpp"
#include "geometry/OBJLoader.hpp"
#include <iostream>
#include <filesystem>
#include <cmath>
#include <glm/gtc/matrix_transform.hpp>

namespace fs = std::filesystem;

namespace world {

static geometry::SubMesh createBoxSubMesh(float hx, float hy, float hz) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Box";

    // 24 vértices para 6 caras con normales ortogonales precisas
    // Cara +Y (Superior)
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara -Y (Inferior)
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara +Z (Frontal)
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy,  hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f} });
    // Cara -Z (Trasera)
    mesh.vertices.push_back({ { hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {0.0f, 0.0f, -1.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {0.0f, 0.0f, -1.0f}, {0.0f, 1.0f} });
    // Cara +X (Derecha)
    mesh.vertices.push_back({ { hx, -hy,  hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara -X (Izquierda)
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {-1.0f, 0.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {-1.0f, 0.0f, 0.0f}, {0.0f, 1.0f} });

    for (int i = 0; i < 6; ++i) {
        int base = i * 4;
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 2);
        mesh.indices.push_back(base + 3);
        mesh.indices.push_back(base + 0);
    }
    return mesh;
}

static geometry::SubMesh createStarSubMesh(float outerRadius, float innerRadius, float depth) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Star";

    const int NUM_POINTS = 5;
    const float PI = 3.14159265358979323846f;
    std::vector<glm::vec3> perimeter(NUM_POINTS * 2);

    for (int i = 0; i < NUM_POINTS; ++i) {
        // Punta exterior
        float angleTip = (PI * 0.5f) + (static_cast<float>(i) * 2.0f * PI / static_cast<float>(NUM_POINTS));
        perimeter[i * 2] = glm::vec3(outerRadius * std::cos(angleTip), outerRadius * std::sin(angleTip), 0.0f);

        // Valle interior
        float angleValley = (PI * 0.5f) + ((static_cast<float>(i) + 0.5f) * 2.0f * PI / static_cast<float>(NUM_POINTS));
        perimeter[i * 2 + 1] = glm::vec3(innerRadius * std::cos(angleValley), innerRadius * std::sin(angleValley), 0.0f);
    }

    glm::vec3 frontApex(0.0f, 0.0f,  depth);
    glm::vec3 backApex(0.0f, 0.0f, -depth);

    auto addTriangle = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 normal = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        uint32_t baseIdx = static_cast<uint32_t>(mesh.vertices.size());

        auto makeUV = [&](const glm::vec3& p) -> glm::vec2 {
            return glm::vec2(0.5f + (p.x / (outerRadius * 2.0f)), 0.5f + (p.y / (outerRadius * 2.0f)));
        };

        mesh.vertices.push_back({ v0, normal, makeUV(v0) });
        mesh.vertices.push_back({ v1, normal, makeUV(v1) });
        mesh.vertices.push_back({ v2, normal, makeUV(v2) });

        mesh.indices.push_back(baseIdx + 0);
        mesh.indices.push_back(baseIdx + 1);
        mesh.indices.push_back(baseIdx + 2);
    };

    // 10 Caras frontales
    for (int i = 0; i < NUM_POINTS * 2; ++i) {
        int next = (i + 1) % (NUM_POINTS * 2);
        addTriangle(frontApex, perimeter[i], perimeter[next]);
    }

    // 10 Caras traseras
    for (int i = 0; i < NUM_POINTS * 2; ++i) {
        int next = (i + 1) % (NUM_POINTS * 2);
        addTriangle(backApex, perimeter[next], perimeter[i]);
    }

    return mesh;
}

static geometry::SubMesh createFacetedDomeSubMesh(float radius, float height, int rings = 3, int segments = 8) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Faceted_Dome";
    const float PI = 3.14159265358979323846f;

    glm::vec3 apex(0.0f, height, 0.0f);

    std::vector<std::vector<glm::vec3>> ringVerts(rings);
    for (int r = 0; r < rings; ++r) {
        float phi = ((r + 1.0f) / static_cast<float>(rings + 1)) * (PI * 0.5f);
        float y = height * std::cos(phi);
        float ringRad = radius * std::sin(phi);
        for (int s = 0; s < segments; ++s) {
            float theta = (static_cast<float>(s) / static_cast<float>(segments)) * (2.0f * PI);
            float bump = 1.0f + 0.07f * std::sin(theta * 4.0f) * std::cos(phi * 3.0f);
            ringVerts[r].push_back(glm::vec3(
                ringRad * bump * std::cos(theta),
                y,
                ringRad * bump * std::sin(theta)
            ));
        }
    }

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x / (radius * 2.0f), 0.5f + p.z / (radius * 2.0f));
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    // Apex to ring 0
    for (int s = 0; s < segments; ++s) {
        int next = (s + 1) % segments;
        addTri(apex, ringVerts[0][s], ringVerts[0][next]);
    }

    // Between rings
    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < segments; ++s) {
            int next = (s + 1) % segments;
            addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][next]);
            addTri(ringVerts[r][s], ringVerts[r+1][next], ringVerts[r][next]);
        }
    }

    // Bottom flat cap at y = 0
    glm::vec3 bottomCenter(0.0f, 0.0f, 0.0f);
    int last = rings - 1;
    for (int s = 0; s < segments; ++s) {
        int next = (s + 1) % segments;
        addTri(bottomCenter, ringVerts[last][next], ringVerts[last][s]);
    }

    return mesh;
}

static geometry::SubMesh createBrazierSubMesh(float baseRadius, float bowlRadius, float totalHeight) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Fire_Brazier";
    const float PI = 3.14159265358979323846f;
    const int segments = 8; // Octágono de hierro forjado

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) norm = glm::vec3(0.0f, 1.0f, 0.0f);
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x / (bowlRadius * 2.0f), 0.5f + p.z / (bowlRadius * 2.0f));
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    struct Ring { float y; float r; };
    std::vector<Ring> rings = {
        { -totalHeight * 0.50f, baseRadius * 1.25f },
        { -totalHeight * 0.38f, baseRadius * 1.00f },
        {  totalHeight * 0.15f, baseRadius * 0.65f },
        {  totalHeight * 0.32f, baseRadius * 0.90f },
        {  totalHeight * 0.50f, bowlRadius }
    };

    std::vector<std::vector<glm::vec3>> ringVerts(rings.size());
    for (size_t r = 0; r < rings.size(); ++r) {
        for (int s = 0; s < segments; ++s) {
            float theta = (static_cast<float>(s) / static_cast<float>(segments)) * 2.0f * PI;
            ringVerts[r].push_back(glm::vec3(
                rings[r].r * std::cos(theta),
                rings[r].y,
                rings[r].r * std::sin(theta)
            ));
        }
    }

    for (size_t r = 0; r < rings.size() - 1; ++r) {
        for (int s = 0; s < segments; ++s) {
            int nxt = (s + 1) % segments;
            addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][nxt]);
            addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r][nxt]);
        }
    }

    glm::vec3 bottomCenter(0.0f, -totalHeight * 0.50f, 0.0f);
    for (int s = 0; s < segments; ++s) {
        int nxt = (s + 1) % segments;
        addTri(bottomCenter, ringVerts[0][nxt], ringVerts[0][s]);
    }

    glm::vec3 innerCoalCenter(0.0f, totalHeight * 0.35f, 0.0f);
    for (int s = 0; s < segments; ++s) {
        int nxt = (s + 1) % segments;
        addTri(innerCoalCenter, ringVerts.back()[s], ringVerts.back()[nxt]);
    }

    return mesh;
}

static geometry::SubMesh createFlameSubMesh(float radius, float height, bool isCore = false) {
    geometry::SubMesh mesh;
    mesh.name = isCore ? "Procedural_Flame_Core" : "Procedural_Volcano_Flame";
    const float PI = 3.14159265358979323846f;

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) norm = glm::vec3(0.0f, 1.0f, 0.0f);
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x / (radius * 2.0f), p.y / height);
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    if (isCore) {
        // Núcleo caliente interno de fuego (forma estilizada de lágrima ardiente)
        struct CoreRing { float y; float r; };
        std::vector<CoreRing> rings = {
            { 0.00f * height, 0.25f * radius },
            { 0.25f * height, 0.85f * radius },
            { 0.55f * height, 0.60f * radius },
            { 0.80f * height, 0.28f * radius }
        };
        const int segments = 8;
        std::vector<std::vector<glm::vec3>> ringVerts(rings.size());
        for (size_t r = 0; r < rings.size(); ++r) {
            for (int s = 0; s < segments; ++s) {
                float th = (static_cast<float>(s) / static_cast<float>(segments)) * 2.0f * PI;
                ringVerts[r].push_back(glm::vec3(
                    rings[r].r * std::cos(th),
                    rings[r].y,
                    rings[r].r * std::sin(th)
                ));
            }
        }

        for (size_t r = 0; r < rings.size() - 1; ++r) {
            for (int s = 0; s < segments; ++s) {
                int nxt = (s + 1) % segments;
                addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][nxt]);
                addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r][nxt]);
            }
        }

        glm::vec3 apex(0.0f, height, 0.0f);
        size_t last = rings.size() - 1;
        for (int s = 0; s < segments; ++s) {
            int nxt = (s + 1) % segments;
            addTri(ringVerts[last][s], apex, ringVerts[last][nxt]);
        }

        glm::vec3 bottom(0.0f, 0.0f, 0.0f);
        for (int s = 0; s < segments; ++s) {
            int nxt = (s + 1) % segments;
            addTri(bottom, ringVerts[0][nxt], ringVerts[0][s]);
        }
    } else {
        // Lenguas de fuego exteriores estilizadas y curvadas con puntas afiladas
        const int numTongues = 5;
        for (int t = 0; t < numTongues; ++t) {
            float ang = static_cast<float>(t) * (2.0f * PI / static_cast<float>(numTongues));
            float thHeight = height * (0.75f + 0.20f * std::sin(static_cast<float>(t) * 1.8f));

            glm::vec3 bL(radius * 0.40f * std::cos(ang - 0.35f), 0.0f, radius * 0.40f * std::sin(ang - 0.35f));
            glm::vec3 bR(radius * 0.40f * std::cos(ang + 0.35f), 0.0f, radius * 0.40f * std::sin(ang + 0.35f));
            glm::vec3 mL(radius * 0.95f * std::cos(ang - 0.20f + 0.15f), thHeight * 0.45f, radius * 0.95f * std::sin(ang - 0.20f + 0.15f));
            glm::vec3 mR(radius * 0.95f * std::cos(ang + 0.20f + 0.15f), thHeight * 0.45f, radius * 0.95f * std::sin(ang + 0.20f + 0.15f));
            glm::vec3 tip(radius * 0.25f * std::cos(ang + 0.35f), thHeight, radius * 0.25f * std::sin(ang + 0.35f));

            // Cara exterior
            addTri(bL, mL, mR);
            addTri(bL, mR, bR);
            addTri(mL, tip, mR);

            // Cara interior (doble cara para visibilidad 360°)
            addTri(mL, bL, mR);
            addTri(mR, bL, bR);
            addTri(tip, mL, mR);
        }
    }

    return mesh;
}

static geometry::SubMesh createSphereSubMesh(float radius, int rings = 6, int sectors = 10) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Sphere";
    const float PI = 3.14159265358979323846f;

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) norm = glm::vec3(0.0f, 1.0f, 0.0f);
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x / (radius * 2.0f), 0.5f + p.z / (radius * 2.0f));
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    glm::vec3 northPole(0.0f, radius, 0.0f);
    glm::vec3 southPole(0.0f, -radius, 0.0f);

    std::vector<std::vector<glm::vec3>> ringVerts;
    for (int r = 1; r < rings; ++r) {
        float phi = PI * static_cast<float>(r) / static_cast<float>(rings);
        std::vector<glm::vec3> rv;
        for (int s = 0; s < sectors; ++s) {
            float th = 2.0f * PI * static_cast<float>(s) / static_cast<float>(sectors);
            rv.push_back(glm::vec3(
                radius * std::sin(phi) * std::cos(th),
                radius * std::cos(phi),
                radius * std::sin(phi) * std::sin(th)
            ));
        }
        ringVerts.push_back(rv);
    }

    for (int s = 0; s < sectors; ++s) {
        int nxt = (s + 1) % sectors;
        addTri(northPole, ringVerts[0][s], ringVerts[0][nxt]);
    }

    for (size_t r = 0; r < ringVerts.size() - 1; ++r) {
        for (int s = 0; s < sectors; ++s) {
            int nxt = (s + 1) % sectors;
            addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][nxt]);
            addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r][nxt]);
        }
    }

    size_t last = ringVerts.size() - 1;
    for (int s = 0; s < sectors; ++s) {
        int nxt = (s + 1) % sectors;
        addTri(southPole, ringVerts[last][nxt], ringVerts[last][s]);
    }

    return mesh;
}

static geometry::SubMesh createClamShellSubMesh(float radius, float openAngleDeg = 50.0f) {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Clam_Shell";
    const float PI = 3.14159265358979323846f;

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) norm = glm::vec3(0.0f, 1.0f, 0.0f);
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x / (radius * 2.0f), 0.5f + p.z / (radius * 2.0f));
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    const int segments = 12;
    glm::vec3 hinge(0.0f, 0.05f, -radius * 0.45f);
    float radAngle = glm::radians(openAngleDeg);
    float cosT = std::cos(radAngle);
    float sinT = std::sin(radAngle);

    for (int valve = 0; valve < 2; ++valve) {
        std::vector<glm::vec3> rimOuter;
        std::vector<glm::vec3> rimInner;

        for (int s = 0; s <= segments; ++s) {
            float t = (static_cast<float>(s) / static_cast<float>(segments)) * PI;
            float angle = t - PI * 0.5f;
            float rib = 1.0f + 0.10f * std::cos(angle * 7.0f);
            float lx = radius * rib * std::cos(angle);
            float lz = radius * rib * std::sin(angle) * 0.75f;

            float distSq = (lx * lx + lz * lz) / (radius * radius);
            float curveY = 0.22f * (1.0f - std::min(1.0f, distSq));

            if (valve == 0) {
                // Valva inferior cóncava
                float yOut = -curveY;
                float yIn  = -curveY * 0.65f + 0.02f;
                rimOuter.push_back(glm::vec3(lx, yOut, lz));
                rimInner.push_back(glm::vec3(lx * 0.92f, yIn, lz * 0.92f));
            } else {
                // Valva superior abierta e inclinada en la bisagra
                float yOut = curveY + 0.06f;
                float yIn  = curveY * 0.65f + 0.04f;

                float zRel = lz - hinge.z;
                float yRot = yOut * cosT - zRel * sinT;
                float zRot = yOut * sinT + zRel * cosT + hinge.z;
                rimOuter.push_back(glm::vec3(lx, yRot, zRot));

                float zRelIn = (lz * 0.92f) - hinge.z;
                float yRotIn = yIn * cosT - zRelIn * sinT;
                float zRotIn = yIn * sinT + zRelIn * cosT + hinge.z;
                rimInner.push_back(glm::vec3(lx * 0.92f, yRotIn, zRotIn));
            }
        }

        glm::vec3 h(hinge.x, hinge.y + (valve == 1 ? 0.06f : 0.0f), hinge.z);
        for (int s = 0; s < segments; ++s) {
            if (valve == 0) {
                addTri(h, rimOuter[s], rimOuter[s+1]);
                addTri(h, rimInner[s+1], rimInner[s]);
            } else {
                addTri(h, rimOuter[s+1], rimOuter[s]);
                addTri(h, rimInner[s], rimInner[s+1]);
            }
        }
    }

    return mesh;
}

static geometry::SubMesh createBubbleColumnSubMesh() {
    geometry::SubMesh mesh;
    mesh.name = "Procedural_Bubble_Column";
    const float PI = 3.14159265358979323846f;

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) norm = glm::vec3(0.0f, 1.0f, 0.0f);
        uint32_t base = static_cast<uint32_t>(mesh.vertices.size());
        auto uv = [&](const glm::vec3& p) {
            return glm::vec2(0.5f + p.x * 0.5f, p.y * 0.25f);
        };
        mesh.vertices.push_back({ v0, norm, uv(v0) });
        mesh.vertices.push_back({ v1, norm, uv(v1) });
        mesh.vertices.push_back({ v2, norm, uv(v2) });
        mesh.indices.push_back(base + 0);
        mesh.indices.push_back(base + 1);
        mesh.indices.push_back(base + 2);
    };

    auto addSphere = [&](const glm::vec3& center, float radius, int rings = 4, int sectors = 6) {
        glm::vec3 northPole = center + glm::vec3(0.0f, radius, 0.0f);
        glm::vec3 southPole = center - glm::vec3(0.0f, radius, 0.0f);

        std::vector<std::vector<glm::vec3>> ringVerts;
        for (int r = 1; r < rings; ++r) {
            float phi = PI * static_cast<float>(r) / static_cast<float>(rings);
            std::vector<glm::vec3> rv;
            for (int s = 0; s < sectors; ++s) {
                float th = 2.0f * PI * static_cast<float>(s) / static_cast<float>(sectors);
                rv.push_back(center + glm::vec3(
                    radius * std::sin(phi) * std::cos(th),
                    radius * std::cos(phi),
                    radius * std::sin(phi) * std::sin(th)
                ));
            }
            ringVerts.push_back(rv);
        }

        for (int s = 0; s < sectors; ++s) {
            int nxt = (s + 1) % sectors;
            addTri(northPole, ringVerts[0][s], ringVerts[0][nxt]);
        }

        for (size_t r = 0; r < ringVerts.size() - 1; ++r) {
            for (int s = 0; s < sectors; ++s) {
                int nxt = (s + 1) % sectors;
                addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][nxt]);
                addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r][nxt]);
            }
        }

        size_t last = ringVerts.size() - 1;
        for (int s = 0; s < sectors; ++s) {
            int nxt = (s + 1) % sectors;
            addTri(southPole, ringVerts[last][nxt], ringVerts[last][s]);
        }
    };

    // Columna de 9 burbujas esféricas ascendentes con offsets orgánicos
    struct BubbleSpec { glm::vec3 pos; float rad; };
    std::vector<BubbleSpec> bubbles = {
        { {  0.05f, 0.30f,  0.02f }, 0.12f },
        { { -0.08f, 0.75f, -0.04f }, 0.16f },
        { {  0.09f, 1.25f,  0.06f }, 0.10f },
        { { -0.04f, 1.70f,  0.08f }, 0.18f },
        { {  0.07f, 2.20f, -0.06f }, 0.13f },
        { { -0.06f, 2.65f, -0.02f }, 0.15f },
        { {  0.05f, 3.10f,  0.07f }, 0.11f },
        { { -0.03f, 3.55f, -0.05f }, 0.17f },
        { {  0.02f, 3.95f,  0.03f }, 0.14f }
    };

    for (const auto& b : bubbles) {
        addSphere(b.pos, b.rad, 4, 6);
    }

    return mesh;
}

PropSpawner::PropSpawner() = default;
PropSpawner::~PropSpawner() = default;

bool PropSpawner::init(const std::string& obstaclesDir) {
    std::vector<std::string> modelFiles = {
        "cone.obj", "barrierRed.obj", "barrierWhite.obj", "barrierWall.obj",
        "box.obj", "debris-tire.obj"
    };

    for (const auto& file : modelFiles) {
        std::string fullPath = (fs::path(obstaclesDir) / file).string();
        geometry::ModelData data;
        if (geometry::OBJLoader::load(fullPath, data) && !data.submeshes.empty()) {
            std::string key = fs::path(file).stem().string();
            renderer::MeshGL mesh;
            mesh.setup(data.submeshes[0]);
            m_propMeshes[key] = std::move(mesh);
            std::cout << "[OK] [PropSpawner] Prop cargado: " << key << std::endl;
        } else {
            std::cerr << "[AVISO] [PropSpawner] No se pudo cargar prop: " << fullPath << std::endl;
        }
    }

    // 2. Mallas procedurales adicionales
    {
        // Farolas 3D: Mástil vertical, brazo y bombilla
        renderer::MeshGL poleMesh;
        poleMesh.setup(createBoxSubMesh(0.12f, 2.1f, 0.12f));
        m_propMeshes["lampPole"] = std::move(poleMesh);

        renderer::MeshGL armMesh;
        armMesh.setup(createBoxSubMesh(0.10f, 0.08f, 0.70f));
        m_propMeshes["lampArm"] = std::move(armMesh);

        renderer::MeshGL bulbMesh;
        bulbMesh.setup(createBoxSubMesh(0.32f, 0.18f, 0.32f));
        m_propMeshes["lampBulb"] = std::move(bulbMesh);

        // Torres de iluminación de estadio
        renderer::MeshGL towerMesh;
        towerMesh.setup(createBoxSubMesh(0.35f, 5.0f, 0.35f));
        m_propMeshes["stadiumTower"] = std::move(towerMesh);

        // Árboles y pinos procedurales de estilo Low-Poly
        renderer::MeshGL trunkMesh;
        trunkMesh.setup(createBoxSubMesh(0.18f, 0.8f, 0.18f));
        m_propMeshes["treeTrunk"] = std::move(trunkMesh);

        renderer::MeshGL folBottomMesh;
        folBottomMesh.setup(createBoxSubMesh(0.85f, 0.55f, 0.85f));
        m_propMeshes["treeFoliageBottom"] = std::move(folBottomMesh);

        renderer::MeshGL folTopMesh;
        folTopMesh.setup(createBoxSubMesh(0.55f, 0.50f, 0.55f));
        m_propMeshes["treeFoliageTop"] = std::move(folTopMesh);

        // Arco de Meta / Gantry pórtico sobre la pista (dimensiones ampliadas para apoyo perfecto)
        renderer::MeshGL gantryPillar;
        gantryPillar.setup(createBoxSubMesh(0.16f, 2.30f, 0.16f));
        m_propMeshes["gantryPillar"] = std::move(gantryPillar);

        renderer::MeshGL gantryFooting;
        gantryFooting.setup(createBoxSubMesh(0.28f, 0.18f, 0.28f));
        m_propMeshes["gantryFooting"] = std::move(gantryFooting);

        renderer::MeshGL gantryBeam;
        gantryBeam.setup(createBoxSubMesh(2.70f, 0.16f, 0.20f));
        m_propMeshes["gantryBeam"] = std::move(gantryBeam);

        renderer::MeshGL gantryLowerBeam;
        gantryLowerBeam.setup(createBoxSubMesh(2.40f, 0.08f, 0.12f));
        m_propMeshes["gantryLowerBeam"] = std::move(gantryLowerBeam);

        renderer::MeshGL gantrySign;
        gantrySign.setup(createBoxSubMesh(1.65f, 0.35f, 0.06f));
        m_propMeshes["gantrySign"] = std::move(gantrySign);

        renderer::MeshGL gantryLight;
        gantryLight.setup(createBoxSubMesh(0.12f, 0.12f, 0.12f));
        m_propMeshes["gantryLight"] = std::move(gantryLight);

        // Elementos exclusivos para circuitos nocturnos (Neon Arches y Balizas)
        renderer::MeshGL neonArchPillar;
        neonArchPillar.setup(createBoxSubMesh(0.16f, 2.30f, 0.16f));
        m_propMeshes["neonArchPillar"] = std::move(neonArchPillar);

        renderer::MeshGL neonArchBeam;
        neonArchBeam.setup(createBoxSubMesh(2.70f, 0.16f, 0.20f));
        m_propMeshes["neonArchBeam"] = std::move(neonArchBeam);

        renderer::MeshGL neonArchTrimCyan;
        neonArchTrimCyan.setup(createBoxSubMesh(2.70f, 0.06f, 0.24f));
        m_propMeshes["neonArchTrimCyan"] = std::move(neonArchTrimCyan);

        renderer::MeshGL neonArchTrimMagenta;
        neonArchTrimMagenta.setup(createBoxSubMesh(2.70f, 0.06f, 0.24f));
        m_propMeshes["neonArchTrimMagenta"] = std::move(neonArchTrimMagenta);

        renderer::MeshGL neonBeacon;
        neonBeacon.setup(createBoxSubMesh(0.15f, 0.40f, 0.15f));
        m_propMeshes["neonBeacon"] = std::move(neonBeacon);

        renderer::MeshGL neonBeaconTop;
        neonBeaconTop.setup(createBoxSubMesh(0.18f, 0.12f, 0.18f));
        m_propMeshes["neonBeaconTop"] = std::move(neonBeaconTop);

        // Elementos procedurales del Desierto (Cactus saguaro 3D y rocas de arenisca)
        renderer::MeshGL cactusTrunk;
        cactusTrunk.setup(createBoxSubMesh(0.18f, 1.25f, 0.18f));
        m_propMeshes["cactusTrunk"] = std::move(cactusTrunk);

        renderer::MeshGL cactusArmJoint;
        cactusArmJoint.setup(createBoxSubMesh(0.24f, 0.10f, 0.10f));
        m_propMeshes["cactusArmJoint"] = std::move(cactusArmJoint);

        renderer::MeshGL cactusArmSpire;
        cactusArmSpire.setup(createBoxSubMesh(0.12f, 0.50f, 0.12f));
        m_propMeshes["cactusArmSpire"] = std::move(cactusArmSpire);

        renderer::MeshGL desertRock;
        desertRock.setup(createBoxSubMesh(0.65f, 0.55f, 0.65f));
        m_propMeshes["desertRock"] = std::move(desertRock);

        renderer::MeshGL desertMesaRock;
        desertMesaRock.setup(createBoxSubMesh(1.20f, 0.85f, 1.10f));
        m_propMeshes["desertMesaRock"] = std::move(desertMesaRock);

        renderer::MeshGL desertArchPillar;
        desertArchPillar.setup(createBoxSubMesh(0.40f, 2.50f, 0.40f));
        m_propMeshes["desertArchPillar"] = std::move(desertArchPillar);

        renderer::MeshGL desertArchBeam;
        desertArchBeam.setup(createBoxSubMesh(2.90f, 0.35f, 0.45f));
        m_propMeshes["desertArchBeam"] = std::move(desertArchBeam);

        // Elementos procedurales del Bioma Espacial (Warp Rings, Satélites y Balizas Galácticas)
        renderer::MeshGL spaceWarpPillar;
        spaceWarpPillar.setup(createBoxSubMesh(0.18f, 2.40f, 0.18f));
        m_propMeshes["spaceWarpPillar"] = std::move(spaceWarpPillar);

        renderer::MeshGL spaceWarpBeam;
        spaceWarpBeam.setup(createBoxSubMesh(2.65f, 0.15f, 0.20f));
        m_propMeshes["spaceWarpBeam"] = std::move(spaceWarpBeam);

        renderer::MeshGL spaceWarpTrimCyan;
        spaceWarpTrimCyan.setup(createBoxSubMesh(2.65f, 0.08f, 0.24f));
        m_propMeshes["spaceWarpTrimCyan"] = std::move(spaceWarpTrimCyan);

        renderer::MeshGL spaceWarpTrimMagenta;
        spaceWarpTrimMagenta.setup(createBoxSubMesh(2.65f, 0.08f, 0.24f));
        m_propMeshes["spaceWarpTrimMagenta"] = std::move(spaceWarpTrimMagenta);

        renderer::MeshGL satelliteBody;
        satelliteBody.setup(createBoxSubMesh(0.40f, 0.40f, 0.40f));
        m_propMeshes["satelliteBody"] = std::move(satelliteBody);

        renderer::MeshGL satellitePanel;
        satellitePanel.setup(createBoxSubMesh(1.10f, 0.04f, 0.35f));
        m_propMeshes["satellitePanel"] = std::move(satellitePanel);

        renderer::MeshGL satelliteAntenna;
        satelliteAntenna.setup(createBoxSubMesh(0.04f, 0.70f, 0.04f));
        m_propMeshes["satelliteAntenna"] = std::move(satelliteAntenna);

        renderer::MeshGL spaceBeacon;
        spaceBeacon.setup(createBoxSubMesh(0.16f, 0.45f, 0.16f));
        m_propMeshes["spaceBeacon"] = std::move(spaceBeacon);

        renderer::MeshGL spaceBeaconTop;
        spaceBeaconTop.setup(createBoxSubMesh(0.20f, 0.14f, 0.20f));
        m_propMeshes["spaceBeaconTop"] = std::move(spaceBeaconTop);

        // Estrella 3D Neón procedural para el bioma espacial (joya estelar facetada)
        renderer::MeshGL starMesh;
        starMesh.setup(createStarSubMesh(0.55f, 0.22f, 0.18f));
        m_propMeshes["floatingStar"] = std::move(starMesh);

        // Elementos procedurales del Bioma Acuático (Corales 3D, algas marinas, rocas submarinas, arco atlante y perlas)
        renderer::MeshGL coralBranchTall;
        coralBranchTall.setup(createBoxSubMesh(0.14f, 1.10f, 0.14f));
        m_propMeshes["coralBranchTall"] = std::move(coralBranchTall);

        renderer::MeshGL coralBranchSide;
        coralBranchSide.setup(createBoxSubMesh(0.12f, 0.50f, 0.12f));
        m_propMeshes["coralBranchSide"] = std::move(coralBranchSide);

        renderer::MeshGL coralBrain;
        coralBrain.setup(createFacetedDomeSubMesh(0.52f, 0.45f, 3, 8));
        m_propMeshes["coralBrain"] = std::move(coralBrain);

        renderer::MeshGL kelpSegment;
        kelpSegment.setup(createBoxSubMesh(0.24f, 0.85f, 0.06f));
        m_propMeshes["kelpSegment"] = std::move(kelpSegment);

        renderer::MeshGL bubbleColumn;
        bubbleColumn.setup(createBubbleColumnSubMesh());
        m_propMeshes["bubbleColumn"] = std::move(bubbleColumn);

        renderer::MeshGL seaRock;
        seaRock.setup(createBoxSubMesh(0.70f, 0.55f, 0.70f));
        m_propMeshes["seaRock"] = std::move(seaRock);

        renderer::MeshGL seaArchPillar;
        seaArchPillar.setup(createBoxSubMesh(0.22f, 2.50f, 0.22f));
        m_propMeshes["seaArchPillar"] = std::move(seaArchPillar);

        renderer::MeshGL seaArchBeam;
        seaArchBeam.setup(createBoxSubMesh(2.80f, 0.25f, 0.30f));
        m_propMeshes["seaArchBeam"] = std::move(seaArchBeam);

        renderer::MeshGL seaArchTrim;
        seaArchTrim.setup(createBoxSubMesh(2.80f, 0.08f, 0.36f));
        m_propMeshes["seaArchTrim"] = std::move(seaArchTrim);

        renderer::MeshGL seaPedestal;
        seaPedestal.setup(createBoxSubMesh(0.22f, 0.75f, 0.22f));
        m_propMeshes["seaPedestal"] = std::move(seaPedestal);

        // Ostra marina abierta para albergar la perla bioluminiscente
        renderer::MeshGL clamShell;
        clamShell.setup(createClamShellSubMesh(0.50f, 50.0f));
        m_propMeshes["clamShell"] = std::move(clamShell);

        // Perla marina esférica bioluminiscente perfecta (reemplaza la estrella)
        renderer::MeshGL seaPearl;
        seaPearl.setup(createSphereSubMesh(0.24f, 6, 10));
        m_propMeshes["seaPearl"] = std::move(seaPearl);

        // Elementos procedurales del Bioma Volcánico (Basalto negro, magma, chimeneas y braseros de fuego)
        renderer::MeshGL volcanoArchPillar;
        volcanoArchPillar.setup(createBoxSubMesh(0.24f, 2.60f, 0.24f));
        m_propMeshes["volcanoArchPillar"] = std::move(volcanoArchPillar);

        renderer::MeshGL volcanoArchBeam;
        volcanoArchBeam.setup(createBoxSubMesh(2.80f, 0.25f, 0.32f));
        m_propMeshes["volcanoArchBeam"] = std::move(volcanoArchBeam);

        renderer::MeshGL volcanoArchTrimLava;
        volcanoArchTrimLava.setup(createBoxSubMesh(2.80f, 0.08f, 0.36f));
        m_propMeshes["volcanoArchTrimLava"] = std::move(volcanoArchTrimLava);

        renderer::MeshGL basaltColumn;
        basaltColumn.setup(createBoxSubMesh(0.35f, 1.60f, 0.35f));
        m_propMeshes["basaltColumn"] = std::move(basaltColumn);

        renderer::MeshGL volcanoRock;
        volcanoRock.setup(createBoxSubMesh(0.65f, 0.50f, 0.65f));
        m_propMeshes["volcanoRock"] = std::move(volcanoRock);

        renderer::MeshGL lavaChimney;
        lavaChimney.setup(createBoxSubMesh(0.26f, 1.10f, 0.26f));
        m_propMeshes["lavaChimney"] = std::move(lavaChimney);

        renderer::MeshGL lavaVent;
        lavaVent.setup(createBoxSubMesh(0.40f, 0.08f, 0.40f));
        m_propMeshes["lavaVent"] = std::move(lavaVent);

        renderer::MeshGL fireBrazier;
        fireBrazier.setup(createBrazierSubMesh(0.24f, 0.38f, 1.40f));
        m_propMeshes["fireBrazier"] = std::move(fireBrazier);

        renderer::MeshGL volcanoFlame;
        volcanoFlame.setup(createFlameSubMesh(0.36f, 1.05f, false));
        m_propMeshes["volcanoFlame"] = std::move(volcanoFlame);

        renderer::MeshGL volcanoFlameCore;
        volcanoFlameCore.setup(createFlameSubMesh(0.22f, 0.72f, true));
        m_propMeshes["volcanoFlameCore"] = std::move(volcanoFlameCore);

        // Alias flameOrb a volcanoFlame para compatibilidad
        renderer::MeshGL flameOrb;
        flameOrb.setup(createFlameSubMesh(0.36f, 1.05f, false));
        m_propMeshes["flameOrb"] = std::move(flameOrb);
    }

    m_initialized = !m_propMeshes.empty();
    return m_initialized;
}

void PropSpawner::addProp(const std::string& type, const glm::vec3& pos, float rotDeg,
                          const glm::vec3& scale, bool hasCollision,
                          const glm::vec4& customColor, bool useCustomColor) {
    PropInstance inst;
    inst.type = type;
    inst.position = pos;
    inst.rotationDegrees = rotDeg;
    inst.scale = scale;
    inst.hasCollision = hasCollision;
    inst.customColor = customColor;
    inst.useCustomColor = useCustomColor;

    glm::mat4 mat = glm::translate(glm::mat4(1.0f), pos);
    mat = glm::rotate(mat, glm::radians(rotDeg), glm::vec3(0.0f, 1.0f, 0.0f));
    mat = glm::scale(mat, scale);
    inst.modelMatrix = mat;

    // Si no tiene colisión física activa, dejamos los límites en cero para evitar obstáculos fantasma
    if (!hasCollision) {
        inst.aabbMin = glm::vec3(0.0f);
        inst.aabbMax = glm::vec3(0.0f);
        m_props.push_back(inst);
        return;
    }

    // AABBs ajustados con máxima fidelidad a la malla visual (sin buffers sobredimensionados)
    if (type == "cone") {
        // Cono real: radio visual 0.24m, altura 0.60m
        float r = 0.24f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        inst.aabbMax = glm::vec3(pos.x + r, 0.62f * scale.y, pos.z + r);
    } else if (type == "barrierWall") {
        float hx = 0.50f * scale.x;
        float hz = 0.14f * scale.z;
        if (std::abs(rotDeg - 90.0f) < 5.0f || std::abs(rotDeg - 270.0f) < 5.0f) {
            std::swap(hx, hz);
        }
        inst.aabbMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        inst.aabbMax = glm::vec3(pos.x + hx, 0.55f * scale.y, pos.z + hz);
    } else if (type == "barrierRed" || type == "barrierWhite") {
        float hx = 0.32f * scale.x;
        float hz = 0.16f * scale.z;
        inst.aabbMin = glm::vec3(pos.x - hx, 0.0f, pos.z - hz);
        inst.aabbMax = glm::vec3(pos.x + hx, 0.45f * scale.y, pos.z + hz);
    } else if (type == "box") {
        float hs = 0.35f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - hs, 0.0f, pos.z - hs);
        inst.aabbMax = glm::vec3(pos.x + hs, 0.70f * scale.y, pos.z + hs);
    } else if (type == "debris-tire") {
        float r = 0.28f * scale.x;
        inst.aabbMin = glm::vec3(pos.x - r, 0.0f, pos.z - r);
        inst.aabbMax = glm::vec3(pos.x + r, 0.55f * scale.y, pos.z + r);
    } else if (type == "lampPole") {
        inst.aabbMin = glm::vec3(pos.x - 0.15f, 0.0f, pos.z - 0.15f);
        inst.aabbMax = glm::vec3(pos.x + 0.15f, 4.40f * scale.y, pos.z + 0.15f);
    } else if (type == "treeTrunk") {
        inst.aabbMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        inst.aabbMax = glm::vec3(pos.x + 0.20f, 1.80f * scale.y, pos.z + 0.20f);
    } else if (type == "stadiumTower") {
        inst.aabbMin = glm::vec3(pos.x - 0.40f, 0.0f, pos.z - 0.40f);
        inst.aabbMax = glm::vec3(pos.x + 0.40f, 10.0f * scale.y, pos.z + 0.40f);
    } else if (type == "gantryPillar" || type == "neonArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        inst.aabbMax = glm::vec3(pos.x + 0.18f, 4.60f * scale.y, pos.z + 0.18f);
    } else if (type == "neonBeacon") {
        inst.aabbMin = glm::vec3(pos.x - 0.16f, 0.0f, pos.z - 0.16f);
        inst.aabbMax = glm::vec3(pos.x + 0.16f, 0.85f * scale.y, pos.z + 0.16f);
    } else if (type == "cactusTrunk") {
        inst.aabbMin = glm::vec3(pos.x - 0.22f, 0.0f, pos.z - 0.22f);
        inst.aabbMax = glm::vec3(pos.x + 0.22f, 2.60f * scale.y, pos.z + 0.22f);
    } else if (type == "desertRock") {
        inst.aabbMin = glm::vec3(pos.x - 0.65f * scale.x, 0.0f, pos.z - 0.65f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.65f * scale.x, 1.10f * scale.y, pos.z + 0.65f * scale.z);
    } else if (type == "desertMesaRock") {
        inst.aabbMin = glm::vec3(pos.x - 1.20f * scale.x, 0.0f, pos.z - 1.20f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 1.20f * scale.x, 1.70f * scale.y, pos.z + 1.20f * scale.z);
    } else if (type == "desertArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.42f, 0.0f, pos.z - 0.42f);
        inst.aabbMax = glm::vec3(pos.x + 0.42f, 5.0f * scale.y, pos.z + 0.42f);
    } else if (type == "spaceWarpPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.20f, 0.0f, pos.z - 0.20f);
        inst.aabbMax = glm::vec3(pos.x + 0.20f, 4.80f * scale.y, pos.z + 0.20f);
    } else if (type == "spaceBeacon") {
        inst.aabbMin = glm::vec3(pos.x - 0.18f, 0.0f, pos.z - 0.18f);
        inst.aabbMax = glm::vec3(pos.x + 0.18f, 0.90f * scale.y, pos.z + 0.18f);
    } else if (type == "seaArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.24f, 0.0f, pos.z - 0.24f);
        inst.aabbMax = glm::vec3(pos.x + 0.24f, 5.0f * scale.y, pos.z + 0.24f);
    } else if (type == "seaRock") {
        inst.aabbMin = glm::vec3(pos.x - 0.70f * scale.x, 0.0f, pos.z - 0.70f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.70f * scale.x, 1.10f * scale.y, pos.z + 0.70f * scale.z);
    } else if (type == "seaPedestal") {
        inst.aabbMin = glm::vec3(pos.x - 0.24f, 0.0f, pos.z - 0.24f);
        inst.aabbMax = glm::vec3(pos.x + 0.24f, 1.50f * scale.y, pos.z + 0.24f);
    } else if (type == "coralBranchTall" || type == "coralBranchSide") {
        inst.aabbMin = glm::vec3(pos.x - 0.20f * scale.x, 0.0f, pos.z - 0.20f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.20f * scale.x, 2.20f * scale.y, pos.z + 0.20f * scale.z);
    } else if (type == "coralBrain") {
        inst.aabbMin = glm::vec3(pos.x - 0.52f * scale.x, 0.0f, pos.z - 0.52f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.52f * scale.x, 0.55f * scale.y, pos.z + 0.52f * scale.z);
    } else if (type == "volcanoArchPillar") {
        inst.aabbMin = glm::vec3(pos.x - 0.26f, 0.0f, pos.z - 0.26f);
        inst.aabbMax = glm::vec3(pos.x + 0.26f, 5.20f * scale.y, pos.z + 0.26f);
    } else if (type == "basaltColumn") {
        inst.aabbMin = glm::vec3(pos.x - 0.38f * scale.x, 0.0f, pos.z - 0.38f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.38f * scale.x, 3.20f * scale.y, pos.z + 0.38f * scale.z);
    } else if (type == "volcanoRock") {
        inst.aabbMin = glm::vec3(pos.x - 0.68f * scale.x, 0.0f, pos.z - 0.68f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.68f * scale.x, 1.05f * scale.y, pos.z + 0.68f * scale.z);
    } else if (type == "lavaChimney") {
        inst.aabbMin = glm::vec3(pos.x - 0.28f * scale.x, 0.0f, pos.z - 0.28f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.28f * scale.x, 2.20f * scale.y, pos.z + 0.28f * scale.z);
    } else if (type == "fireBrazier") {
        inst.aabbMin = glm::vec3(pos.x - 0.24f * scale.x, 0.0f, pos.z - 0.24f * scale.z);
        inst.aabbMax = glm::vec3(pos.x + 0.24f * scale.x, 1.40f * scale.y, pos.z + 0.24f * scale.z);
    } else {
        // Elementos suspendidos (vigas a 4m de altura, luces, letrero de meta):
        glm::vec3 halfSize = scale * 0.5f;
        inst.aabbMin = pos - halfSize;
        inst.aabbMax = pos + halfSize;
    }

    m_props.push_back(inst);
}

void PropSpawner::update(float dt) {
    m_animTime += dt;
    m_starRotation += dt * 65.0f;
    if (m_starRotation >= 360.0f) {
        m_starRotation -= 360.0f;
    }

    // Animar giro 3D continuo de estrellas flotantes espaciales
    for (auto& prop : m_props) {
        if (prop.type == "floatingStar") {
            float bob = std::sin(m_animTime * 2.8f + prop.position.x * 0.4f + prop.position.z * 0.4f) * 0.08f;
            glm::vec3 animatedPos = prop.position + glm::vec3(0.0f, bob, 0.0f);

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), animatedPos);
            mat = glm::rotate(mat, glm::radians(m_starRotation + prop.rotationDegrees), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::scale(mat, prop.scale);
            prop.modelMatrix = mat;
        } else if (prop.type == "seaPearl") {
            // La perla dentro de la ostra pulsa suavemente con respiración bioluminiscente
            float pulse = 1.0f + 0.045f * std::sin(m_animTime * 2.8f + prop.position.x * 0.5f);
            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position);
            mat = glm::scale(mat, prop.scale * pulse);
            prop.modelMatrix = mat;
        } else if (prop.type == "bubbleColumn") {
            // Las columnas de burbujas ascienden con bamboleo hidrodinámico senoidal
            float phase = prop.position.x * 1.7f + prop.position.z * 1.3f;
            float yAscend = std::fmod(m_animTime * 1.5f + phase, 1.2f) * 0.75f;
            float wobbleX = 0.12f * std::sin(m_animTime * 3.5f + phase);
            float wobbleZ = 0.10f * std::cos(m_animTime * 4.2f + phase);
            float pulse = 1.0f + 0.06f * std::sin(m_animTime * 5.0f + phase);

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position + glm::vec3(wobbleX, yAscend, wobbleZ));
            mat = glm::scale(mat, prop.scale * glm::vec3(pulse, 1.0f, pulse));
            prop.modelMatrix = mat;
        } else if (prop.type == "volcanoFlame" || prop.type == "volcanoFlameCore" || prop.type == "flameOrb") {
            // El fuego volcánico parpadea, vibra y oscila orgánicamente con ráfagas térmicas y viento
            float phase = prop.position.x * 2.3f + prop.position.z * 1.7f;
            float flickerY = 1.0f + 0.15f * std::sin(m_animTime * 14.5f + phase) + 0.07f * std::cos(m_animTime * 23.0f + phase * 1.5f);
            float flickerXZ = 1.0f + 0.08f * std::cos(m_animTime * 11.0f + phase);
            float swayX = 4.5f * std::sin(m_animTime * 5.5f + phase);
            float swayZ = 3.5f * std::cos(m_animTime * 6.5f + phase);
            float slowDrift = m_animTime * 28.0f + prop.rotationDegrees;

            glm::mat4 mat = glm::translate(glm::mat4(1.0f), prop.position);
            mat = glm::rotate(mat, glm::radians(swayX), glm::vec3(1.0f, 0.0f, 0.0f));
            mat = glm::rotate(mat, glm::radians(swayZ), glm::vec3(0.0f, 0.0f, 1.0f));
            mat = glm::rotate(mat, glm::radians(slowDrift), glm::vec3(0.0f, 1.0f, 0.0f));
            mat = glm::scale(mat, prop.scale * glm::vec3(flickerXZ, flickerY, flickerXZ));
            prop.modelMatrix = mat;
        }
    }
}

void PropSpawner::generateForGrid(const TrackGrid& grid, bool isNight, bool isDesert, bool isSpace, bool isAquatic, bool isVolcano) {
    m_props.clear();
    m_lampPositions.clear();

    float halfTile = TrackGrid::TILE_SIZE * 0.5f;
    int propIndex = 0;

    // Recopilar losas de la pista
    std::vector<const TileData*> straightTiles;
    const TileData* startFinishTile = nullptr;

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        } else if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        }
    }

    if (isVolcano) {
        // =========================================================
        // GENERACION BIOMA VOLCANICO: Caldera del Volcán UCV (Inferno Magma)
        // =========================================================

        // 1. Gran Portal del Dragón en Basalto y Magma en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.65f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 pillarAOffset = sideDir * pillarOffset;
            glm::vec3 pillarBOffset = -sideDir * pillarOffset;

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            // Pilares de basalto volcánico negro
            addProp("volcanoArchPillar", glm::vec3(posA.x, 2.60f, posA.z), beamRot, glm::vec3(1.0f), true);
            addProp("volcanoArchPillar", glm::vec3(posB.x, 2.60f, posB.z), beamRot, glm::vec3(1.0f), true);

            // Viga superior con vena de magma incandescente
            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.20f, 0.0f);
            addProp("volcanoArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);
            addProp("volcanoArchTrimLava", beamPos - glm::vec3(0.0f, 0.20f, 0.0f), beamRot, glm::vec3(1.0f), false);

            // Rocas de lava y braseros de fuego en las bases
            addProp("volcanoRock", posA + sideDir * 1.2f, 35.0f, glm::vec3(1.2f), true);
            addProp("volcanoRock", posB - sideDir * 1.2f, 65.0f, glm::vec3(1.2f), true);

            addProp("fireBrazier", posA + sideDir * 1.8f + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);
            addProp("volcanoFlame", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.25f, 0.0f), 0.0f, glm::vec3(1.0f), false);
            addProp("volcanoFlameCore", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.28f, 0.0f), 0.0f, glm::vec3(0.9f), false);

            addProp("fireBrazier", posB - sideDir * 1.8f + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);
            addProp("volcanoFlame", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.25f, 0.0f), 0.0f, glm::vec3(1.0f), false);
            addProp("volcanoFlameCore", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.28f, 0.0f), 0.0f, glm::vec3(0.9f), false);

            m_lampPositions.push_back(posA + sideDir * 1.8f + glm::vec3(0.0f, 1.65f, 0.0f));
            m_lampPositions.push_back(posB - sideDir * 1.8f + glm::vec3(0.0f, 1.65f, 0.0f));
        }

        // 2. Obstáculos y Decoraciones Volcánicas en Curvas
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

                addProp("debris-tire", tile.worldPos + outerDir * 2.40f, tile.rotationDegrees, glm::vec3(1.3f), true);
                addProp("cone", tile.worldPos - outerDir * 1.85f, tile.rotationDegrees + 45.0f, glm::vec3(1.15f), true);

                addProp("basaltColumn", tile.worldPos + outerDir * 3.8f + glm::vec3(0.0f, 1.60f, 0.0f), tile.rotationDegrees + 20.0f, glm::vec3(1.0f), true);
                addProp("volcanoRock", tile.worldPos + outerDir * 5.2f, tile.rotationDegrees + 70.0f, glm::vec3(1.3f), true);
                addProp("lavaVent", tile.worldPos + outerDir * 2.8f + glm::vec3(0.0f, 0.05f, 0.0f), tile.rotationDegrees, glm::vec3(1.0f), false);
            }
        }

        // 3. Columnas de Basalto, Chimeneas de Fumarolas y Braseros en las Rectas
        int flameCount = 0;
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            if (i % 2 == 0) {
                float bDist = 3.6f + (i % 3) * 0.7f;
                glm::vec3 bPos = tile->worldPos + sideDir * bDist;
                float bRot = static_cast<float>((i * 51) % 360);
                addProp("basaltColumn", bPos + glm::vec3(0.0f, 1.60f, 0.0f), bRot, glm::vec3(1.0f), true);
                addProp("volcanoRock", bPos + glm::vec3(0.4f, 0.0f, 0.3f), bRot + 30.0f, glm::vec3(0.9f), true);
            } else {
                float cDist = 3.8f + (i % 2) * 0.6f;
                glm::vec3 cPos = tile->worldPos - sideDir * cDist;
                float cRot = static_cast<float>((i * 37) % 360);
                addProp("lavaChimney", cPos + glm::vec3(0.0f, 1.10f, 0.0f), cRot, glm::vec3(1.0f), true);
                addProp("lavaVent", cPos + glm::vec3(0.2f, 0.05f, 0.2f), cRot, glm::vec3(0.9f), false);
            }

            // Braseros de Fuego Incandescente cada 5 losas (emiten luz puntual de fuego cálido)
            if (i % 5 == 1 && flameCount < 6) {
                float brazierDist = 2.65f;
                glm::vec3 bPos = tile->worldPos + sideDir * brazierDist;
                addProp("fireBrazier", bPos + glm::vec3(0.0f, 0.70f, 0.0f), 0.0f, glm::vec3(1.0f), true);

                float rotDeg = static_cast<float>((i * 53) % 360);
                addProp("volcanoFlame", bPos + glm::vec3(0.0f, 1.25f, 0.0f), rotDeg, glm::vec3(1.0f), false);
                addProp("volcanoFlameCore", bPos + glm::vec3(0.0f, 1.28f, 0.0f), rotDeg, glm::vec3(0.9f), false);

                m_lampPositions.push_back(bPos + glm::vec3(0.0f, 1.65f, 0.0f));
                flameCount++;
            }
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Arco del Dragon, " << flameCount
                  << " braseros de fuego, columnas de basalto y chimeneas volcanicas)." << std::endl;
        return;
    }

    if (isAquatic) {
        // =========================================================
        // GENERACION BIOMA ACUATICO: Arrecife Abisal UCV (Deep Reef)
        // =========================================================

        // 1. Gran Portal Atlante de Coral y Nácar en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.65f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 pillarAOffset = sideDir * pillarOffset;
            glm::vec3 pillarBOffset = -sideDir * pillarOffset;

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            // Pilares de templo atlante marino
            addProp("seaArchPillar", glm::vec3(posA.x, 2.50f, posA.z), beamRot, glm::vec3(1.0f), true);
            addProp("seaArchPillar", glm::vec3(posB.x, 2.50f, posB.z), beamRot, glm::vec3(1.0f), true);

            // Viga superior con ribete nacarado luminoso
            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.00f, 0.0f);
            addProp("seaArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);
            addProp("seaArchTrim", beamPos - glm::vec3(0.0f, 0.20f, 0.0f), beamRot, glm::vec3(1.0f), false);

            // Rocas marinas y perlas resplandecientes en las bases exteriores
            addProp("seaRock", posA + sideDir * 1.2f, 30.0f, glm::vec3(1.2f), true);
            addProp("seaRock", posB - sideDir * 1.2f, 60.0f, glm::vec3(1.2f), true);

            addProp("seaPedestal", posA + sideDir * 1.8f + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);
            addProp("clamShell", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.50f, 0.0f), beamRot + 90.0f, glm::vec3(1.0f), false);
            addProp("seaPearl", posA + sideDir * 1.8f + glm::vec3(0.0f, 1.62f, 0.0f), 0.0f, glm::vec3(1.0f), false);

            addProp("seaPedestal", posB - sideDir * 1.8f + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);
            addProp("clamShell", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.50f, 0.0f), beamRot + 90.0f, glm::vec3(1.0f), false);
            addProp("seaPearl", posB - sideDir * 1.8f + glm::vec3(0.0f, 1.62f, 0.0f), 0.0f, glm::vec3(1.0f), false);

            m_lampPositions.push_back(posA + sideDir * 1.8f + glm::vec3(0.0f, 1.62f, 0.0f));
            m_lampPositions.push_back(posB - sideDir * 1.8f + glm::vec3(0.0f, 1.62f, 0.0f));
        }

        // 2. Obstáculos y Decoraciones del Arrecife en Curvas
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

                addProp("cone", tile.worldPos - outerDir * 1.9f, tile.rotationDegrees + 45.0f, glm::vec3(1.15f), true);
                addProp("debris-tire", tile.worldPos + outerDir * 2.45f, tile.rotationDegrees, glm::vec3(1.3f), true);

                addProp("seaRock", tile.worldPos + outerDir * 4.0f, tile.rotationDegrees + 25.0f, glm::vec3(1.3f), true);
                addProp("coralBrain", tile.worldPos + outerDir * 3.2f + glm::vec3(0.0f, 0.40f, 0.0f), tile.rotationDegrees + 60.0f, glm::vec3(1.2f), true);
                addProp("bubbleColumn", tile.worldPos + outerDir * 5.2f + glm::vec3(0.0f, 1.80f, 0.0f), 0.0f, glm::vec3(1.0f), false);
            }
        }

        // 3. Bosques de Algas Kelp, Ramas de Coral 3D y Pedestales con Perlas en las Rectas
        int pearlCount = 0;
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Algas Kelp marinas altas en los bordes
            float kelpDist = 3.6f + (i % 3) * 0.7f;
            glm::vec3 kelpPos = tile->worldPos + sideDir * kelpDist;
            float kelpRot = static_cast<float>((i * 47) % 360);

            addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 0.85f, 0.0f), kelpRot, glm::vec3(1.0f), false);
            addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 2.35f, 0.0f), kelpRot + 25.0f, glm::vec3(0.9f, 0.85f, 0.9f), false);
            addProp("kelpSegment", kelpPos + glm::vec3(0.0f, 3.75f, 0.0f), kelpRot + 50.0f, glm::vec3(0.8f, 0.80f, 0.8f), false);

            // Racimos de coral 3D (todos con colisión sólida física activa)
            if (i % 2 == 0) {
                glm::vec3 coralPos = tile->worldPos - sideDir * (3.8f + (i % 2) * 0.6f);
                float coralRot = static_cast<float>((i * 63) % 360);
                addProp("coralBranchTall", coralPos + glm::vec3(0.0f, 1.10f, 0.0f), coralRot, glm::vec3(1.0f), true);
                addProp("coralBranchSide", coralPos + glm::vec3(0.25f, 0.75f, 0.0f), coralRot + 45.0f, glm::vec3(1.0f), true);
                addProp("coralBranchSide", coralPos + glm::vec3(-0.25f, 0.95f, 0.0f), coralRot - 45.0f, glm::vec3(1.0f), true);
                addProp("coralBrain", coralPos + glm::vec3(0.6f, 0.40f, 0.3f), coralRot + 90.0f, glm::vec3(0.9f), true);
            }

            // Pedestales con Ostra Abierta y Perla Bioluminiscente cada 5 losas (emiten luz puntual acuática)
            if (i % 5 == 1 && pearlCount < 6) {
                float pearlDist = 2.65f;
                glm::vec3 pPos = tile->worldPos + sideDir * pearlDist;
                addProp("seaPedestal", pPos + glm::vec3(0.0f, 0.75f, 0.0f), 0.0f, glm::vec3(1.0f), true);

                float clamRot = isHorizontal ? 0.0f : 90.0f;
                addProp("clamShell", pPos + glm::vec3(0.0f, 1.50f, 0.0f), clamRot, glm::vec3(1.0f), false);
                addProp("seaPearl", pPos + glm::vec3(0.0f, 1.62f, 0.0f), 0.0f, glm::vec3(1.0f), false);

                m_lampPositions.push_back(pPos + glm::vec3(0.0f, 1.62f, 0.0f));
                pearlCount++;
            }

            // Columnas de burbujas ascendentes adicionales a lo largo de las rectas
            if (i % 4 == 2) {
                addProp("bubbleColumn", tile->worldPos - sideDir * 4.6f + glm::vec3(0.0f, 0.1f, 0.0f), 0.0f, glm::vec3(1.0f), false);
            }
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Arco Atlante, " << pearlCount
                  << " perlas bioluminiscentes, bosques de algas kelp y corales 3D)." << std::endl;
        return;
    }

    if (isSpace) {
        m_starLights.clear();
        // =========================================================
        // GENERACION BIOMA ESPACIAL: Rainbow Cosmic Highway
        // =========================================================

        // 1. Gran Portal de Lanzamiento Orbital en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.45f;
            glm::vec3 pillarAOffset = isHorizontal ? glm::vec3(0.0f, 0.0f,  pillarOffset) : glm::vec3( pillarOffset, 0.0f, 0.0f);
            glm::vec3 pillarBOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, -pillarOffset) : glm::vec3(-pillarOffset, 0.0f, 0.0f);

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot, glm::vec3(1.0f), true);
            addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot, glm::vec3(1.0f), true);

            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.80f, 0.0f);
            addProp("spaceWarpBeam", beamPos, beamRot, glm::vec3(1.0f), false);
            addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.18f, 0.0f), beamRot, glm::vec3(1.0f), false);
            addProp("spaceWarpTrimMagenta", beamPos - glm::vec3(0.0f, 0.18f, 0.0f), beamRot, glm::vec3(1.0f), false);
        }

        // 2. Anillos de Hipervelocidad (Warp Rings) a lo largo de las grandes rectas
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;

            // Cada 6 losas de recta, desplegar un pórtico Warp Ring completo
            if (i % 6 == 2) {
                float pillarOffset = 2.40f;
                glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
                glm::vec3 posA = tile->worldPos + sideDir * pillarOffset;
                glm::vec3 posB = tile->worldPos - sideDir * pillarOffset;

                addProp("spaceWarpPillar", glm::vec3(posA.x, 2.40f, posA.z), beamRot, glm::vec3(1.0f), true);
                addProp("spaceWarpPillar", glm::vec3(posB.x, 2.40f, posB.z), beamRot, glm::vec3(1.0f), true);

                glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 4.70f, 0.0f);
                addProp("spaceWarpBeam", beamPos, beamRot, glm::vec3(1.0f), false);
                if (i % 12 == 2) {
                    addProp("spaceWarpTrimCyan", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot, glm::vec3(1.0f), false);
                } else {
                    addProp("spaceWarpTrimMagenta", beamPos + glm::vec3(0.0f, 0.15f, 0.0f), beamRot, glm::vec3(1.0f), false);
                }
            }

            // Satélites flotantes de comunicación orbital a los lados (suspendidos sin colisión)
            if (i % 8 == 4) {
                float sideSign = (i % 16 == 4) ? 1.0f : -1.0f;
                glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);
                glm::vec3 satPos = tile->worldPos + sideDir * 5.5f + glm::vec3(0.0f, 3.8f, 0.0f);
                float satRot = static_cast<float>((i * 67) % 360);

                addProp("satelliteBody", satPos, satRot, glm::vec3(1.0f), false);
                addProp("satellitePanel", satPos, satRot, glm::vec3(1.0f), false);
                addProp("satelliteAntenna", satPos + glm::vec3(0.0f, 0.55f, 0.0f), satRot, glm::vec3(1.0f), false);
            }
        }

        // 3. Balizas de Navegación Estelar en los bordes de todas las curvas (base sólida, cabezal luminoso)
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));

                glm::vec3 bPos = tile.worldPos + outerDir * 2.30f;
                addProp("spaceBeacon", bPos, tile.rotationDegrees, glm::vec3(1.0f), true);
                addProp("spaceBeaconTop", bPos + glm::vec3(0.0f, 0.45f, 0.0f), tile.rotationDegrees, glm::vec3(1.0f), false);
            }
        }

        // 4. Estrellas Flotantes Neón en la Pista (emiten luces neón vibrantes no-blancas: cian, magenta, oro, verde, violeta)
        struct NeonStarTheme {
            glm::vec4 meshColor;
            glm::vec3 lightColor;
        };

        const std::vector<NeonStarTheme> neonThemes = {
            { glm::vec4(0.0f, 0.98f, 1.0f, 1.0f),  glm::vec3(0.05f, 0.95f, 1.0f) }, // Neon Cian Eléctrico
            { glm::vec4(1.0f, 0.12f, 0.88f, 1.0f), glm::vec3(1.0f, 0.10f, 0.85f) }, // Neon Magenta / Hot Pink
            { glm::vec4(1.0f, 0.85f, 0.08f, 1.0f), glm::vec3(1.0f, 0.80f, 0.08f) }, // Neon Oro / Ámbar Radiante
            { glm::vec4(0.12f, 1.0f, 0.45f, 1.0f), glm::vec3(0.12f, 1.0f, 0.40f) }, // Neon Verde Láser
            { glm::vec4(0.75f, 0.20f, 1.0f, 1.0f), glm::vec3(0.75f, 0.18f, 1.0f) }, // Neon Púrpura Cósmico
            { glm::vec4(0.0f, 0.85f, 0.95f, 1.0f), glm::vec3(0.05f, 0.85f, 0.95f) }, // Neon Turquesa Glaciar
            { glm::vec4(1.0f, 0.45f, 0.05f, 1.0f), glm::vec3(1.0f, 0.42f, 0.05f) }, // Neon Ámbar Solar
            { glm::vec4(0.95f, 0.08f, 0.65f, 1.0f), glm::vec3(0.95f, 0.08f, 0.65f) }  // Neon Fucsia Galáctico
        };

        // Distribuir 8 estrellas flotantes principales espaciadas por todo el circuito
        size_t starCount = 0;
        size_t starStride = std::max<size_t>(1, straightTiles.size() / 8);

        for (size_t i = 0; i < straightTiles.size() && starCount < 8; i += starStride) {
            size_t tileIdx = i;
            if (tileIdx % 6 == 2 && tileIdx + 1 < straightTiles.size()) {
                tileIdx++;
            }
            const auto* sTile = straightTiles[tileIdx];

            bool isHorizontal = (sTile->rotationDegrees == 90.0f || sTile->rotationDegrees == 270.0f);
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);

            // Posicionar en los laterales de la pista (bermas), alternando izquierda y derecha
            float sideSign = (starCount % 2 == 0) ? 1.0f : -1.0f;
            float lateralOffset = 2.45f; // Completamente fuera de la calzada de rodadura (ancho de vía ~4.0m)
            float hoverHeight = 1.65f;   // Flotando a altura de visibilidad lateral del piloto

            glm::vec3 starPos = sTile->worldPos + sideDir * (sideSign * lateralOffset) + glm::vec3(0.0f, hoverHeight, 0.0f);
            const auto& theme = neonThemes[starCount % neonThemes.size()];

            // Se agregan en los laterales sin colisión física para no obstaculizar al kart
            addProp("floatingStar", starPos, static_cast<float>(starCount * 45), glm::vec3(1.0f),
                    false /*hasCollision*/, theme.meshColor, true /*useCustomColor*/);

            m_starLights.push_back({ starPos, theme.lightColor });
            starCount++;
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Portales Warp, satelites, balizas y "
                  << starCount << " estrellas flotantes neon no-blancas)." << std::endl;
        return;
    }

    if (isDesert) {
        // =========================================================
        // GENERACION BIOMA DESIERTO: Rally Dakar / Valle de Dunas
        // =========================================================

        // 1. Arco Natural de Piedra y Arenisca en la Salida / Meta
        if (startFinishTile) {
            bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
            float beamRot = isHorizontal ? 90.0f : 0.0f;
            float pillarOffset = 2.60f; // Mayor holgura para evitar roce con el ancho de pista
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            glm::vec3 pillarAOffset = sideDir * pillarOffset;
            glm::vec3 pillarBOffset = -sideDir * pillarOffset;

            glm::vec3 posA = startFinishTile->worldPos + pillarAOffset;
            glm::vec3 posB = startFinishTile->worldPos + pillarBOffset;

            // Pilares de piedra arenisca natural
            addProp("desertArchPillar", glm::vec3(posA.x, 2.50f, posA.z), beamRot, glm::vec3(1.0f), true);
            addProp("desertArchPillar", glm::vec3(posB.x, 2.50f, posB.z), beamRot, glm::vec3(1.0f), true);

            // Viga superior de roca tallada conectando los pilares (suspendida a 5.1m sin colisión)
            glm::vec3 beamPos = (posA + posB) * 0.5f + glm::vec3(0.0f, 5.10f, 0.0f);
            addProp("desertArchBeam", beamPos, beamRot, glm::vec3(1.0f), false);

            // Monolitos protectores en las bases del arco (ubicados sólidamente fuera de la pista)
            addProp("desertRock", posA + sideDir * 1.1f, 25.0f, glm::vec3(1.1f), true);
            addProp("desertRock", posB - sideDir * 1.1f, 65.0f, glm::vec3(1.1f), true);
        }

        // 2. Obstaculos y decoraciones en curvas: Barreras de neumaticos, conos y rocas de arenisca
        for (const auto& tile : grid.getAllTiles()) {
            if (tile.type == TileType::ROAD_CORNER_SMALL || tile.type == TileType::ROAD_CORNER_LARGE) {
                float rotRad = glm::radians(tile.rotationDegrees);
                glm::vec3 outerDir(std::cos(rotRad + 0.785f), 0.0f, std::sin(rotRad + 0.785f));
                
                // Conos / neumaticos de aviso (con holgura exterior)
                addProp("debris-tire", tile.worldPos + outerDir * 2.35f, tile.rotationDegrees, glm::vec3(1.3f), true);
                addProp("cone", tile.worldPos - outerDir * 1.8f, tile.rotationDegrees + 45.0f, glm::vec3(1.1f), true);

                // Formaciones rocosas naturales de arenisca a los bordes
                addProp("desertRock", tile.worldPos + outerDir * 3.8f, tile.rotationDegrees + 30.0f, glm::vec3(1.3f, 1.2f, 1.3f), true);
                addProp("desertMesaRock", tile.worldPos + outerDir * 5.8f, tile.rotationDegrees + 75.0f, glm::vec3(1.4f, 1.5f, 1.2f), true);
            }
        }

        // 3. Bosques de Cactus Saguaro 3D y Monolitos a lo largo de las rectas
        for (size_t i = 0; i < straightTiles.size(); ++i) {
            const auto* tile = straightTiles[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Generar Cactus Saguaro 3D cada 2 losas
            if (i % 2 == 0) {
                float dist = 3.6f + (i % 3) * 0.8f;
                glm::vec3 cPos = tile->worldPos + sideDir * dist;
                float cactusRot = static_cast<float>((i * 53) % 360);

                // Tronco principal con colisionador sólido
                addProp("cactusTrunk", cPos + glm::vec3(0.0f, 1.25f, 0.0f), cactusRot, glm::vec3(1.0f), true);
                // Brazos del cactus decorativos sin colisión
                float rad = glm::radians(cactusRot);
                glm::vec3 leftDir(-std::sin(rad), 0.0f, std::cos(rad));
                glm::vec3 rightDir(std::sin(rad), 0.0f, -std::cos(rad));

                addProp("cactusArmJoint", cPos + leftDir * 0.28f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot, glm::vec3(1.0f), false);
                addProp("cactusArmSpire", cPos + leftDir * 0.42f + glm::vec3(0.0f, 1.95f, 0.0f), cactusRot, glm::vec3(1.0f), false);

                addProp("cactusArmJoint", cPos + rightDir * 0.28f + glm::vec3(0.0f, 1.15f, 0.0f), cactusRot, glm::vec3(1.0f), false);
                addProp("cactusArmSpire", cPos + rightDir * 0.42f + glm::vec3(0.0f, 1.55f, 0.0f), cactusRot, glm::vec3(1.0f), false);

                // Roca pequeña en la base del cactus
                if (i % 4 == 0) {
                    addProp("desertRock", cPos + sideDir * 0.9f + glm::vec3(0.0f, 0.5f, 0.0f), cactusRot + 45.0f, glm::vec3(0.7f), true);
                }
            } else if (i % 3 == 0) {
                // Rocas de arenisca naturales en los laterales de la pista
                float dist = 4.2f + (i % 2) * 1.2f;
                glm::vec3 rockPos = tile->worldPos - sideDir * dist;
                addProp("desertRock", rockPos + glm::vec3(0.0f, 0.45f, 0.0f), static_cast<float>((i * 71) % 360), glm::vec3(1.1f, 0.8f, 1.0f), true);
            }
        }

        std::cout << "[OK] [PropSpawner] Total de elementos generados: "
                  << m_props.size() << " (Arco natural de arenisca, cactus saguaro 3D y rocas de arenisca del desierto)." << std::endl;
        return;
    }

    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::ROAD_STRAIGHT) {
            straightTiles.push_back(&tile);
        } else if (tile.type == TileType::START_FINISH) {
            startFinishTile = &tile;
        }
    }

    // 1. Arco de Meta / Gantry Monumental sobre la línea de meta
    if (startFinishTile != nullptr) {
        bool isHorizontal = (startFinishTile->rotationDegrees == 90.0f || startFinishTile->rotationDegrees == 270.0f);
        glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
        // Cuando isHorizontal es true, el carril corre por X; los bordes están en Z.
        // Dado que la viga sin rotar se extiende en X, rotarla 90 grados la orienta en Z, conectando ambos pilares
        float beamRot = isHorizontal ? 90.0f : 0.0f;
        float signRot = isHorizontal ? 90.0f : 0.0f;

        // Pilares izquierdo y derecho anclados en el césped a cada lado del carril (4.7m de luz)
        glm::vec3 leftPillar = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        glm::vec3 rightPillar = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
        addProp("gantryPillar", leftPillar, 0.0f, glm::vec3(1.0f), true);
        addProp("gantryPillar", rightPillar, 0.0f, glm::vec3(1.0f), true);

        // Zapatas de hormigón en la base de los pilares (sin colisión separada del pilar)
        glm::vec3 leftFoot = startFinishTile->worldPos + sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        glm::vec3 rightFoot = startFinishTile->worldPos - sideDir * 2.35f + glm::vec3(0.0f, 0.18f, 0.0f);
        addProp("gantryFooting", leftFoot, 0.0f, glm::vec3(1.0f), false);
        addProp("gantryFooting", rightFoot, 0.0f, glm::vec3(1.0f), false);

        // Viga transversal superior en celosía descansando sólidamente sobre los pilares a 4.76m de altura (sin colisión con vehículos)
        glm::vec3 beamPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.76f, 0.0f);
        addProp("gantryBeam", beamPos, beamRot, glm::vec3(1.0f), false);

        // Viga inferior de soporte y unión a 3.90m
        glm::vec3 lowerBeamPos = startFinishTile->worldPos + glm::vec3(0.0f, 3.90f, 0.0f);
        addProp("gantryLowerBeam", lowerBeamPos, beamRot, glm::vec3(1.0f), false);

        // Letrero luminoso de START / FINISH en el centro entre ambas vigas
        glm::vec3 signPos = startFinishTile->worldPos + glm::vec3(0.0f, 4.33f, 0.0f);
        addProp("gantrySign", signPos, signRot, glm::vec3(1.0f), false);

        // 5 Luces de semáforo de salida de carrera F1 suspendidas bajo la estructura
        for (int i = -2; i <= 2; ++i) {
            glm::vec3 lightPos = startFinishTile->worldPos + sideDir * (i * 0.60f) + glm::vec3(0.0f, 3.72f, 0.0f);
            addProp("gantryLight", lightPos, beamRot, glm::vec3(1.0f), false);
        }
    }

    // 2. Generar Farolas 3D Ampliamente Espaciadas (Mínimo 28m de separación entre postes)
    if (!straightTiles.empty()) {
        const float MIN_LAMP_DIST = 28.0f; // Al menos 28 metros entre farolas (7 losas de separación)
        const size_t MAX_LAMPS = 6;        // Máximo 6 farolas estratégicas para no sobrecargar de luz

        std::vector<const TileData*> selectedLamps;
        size_t stride = std::max<size_t>(1, straightTiles.size() / 16);
        for (size_t i = 0; i < straightTiles.size() && selectedLamps.size() < MAX_LAMPS; i += stride) {
            const auto* cand = straightTiles[i];
            bool tooClose = false;
            for (const auto* existing : selectedLamps) {
                if (glm::distance(cand->worldPos, existing->worldPos) < MIN_LAMP_DIST) {
                    tooClose = true;
                    break;
                }
            }
            if (!tooClose) {
                selectedLamps.push_back(cand);
            }
        }

        for (size_t i = 0; i < selectedLamps.size(); ++i) {
            const auto* tile = selectedLamps[i];
            bool isHorizontal = (tile->rotationDegrees == 90.0f || tile->rotationDegrees == 270.0f);
            float sideSign = (i % 2 == 0) ? 1.0f : -1.0f;
            glm::vec3 sideDir = isHorizontal ? glm::vec3(0.0f, 0.0f, sideSign) : glm::vec3(sideSign, 0.0f, 0.0f);

            // Mástil vertical en el césped
            glm::vec3 poleBase = tile->worldPos + sideDir * (halfTile * 1.15f);
            glm::vec3 poleCenter = poleBase + glm::vec3(0.0f, 2.1f, 0.0f);
            addProp("lampPole", poleCenter, 0.0f, glm::vec3(1.0f), true);

            // Base de protección con neumáticos
            addProp("debris-tire", poleBase + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f), true);

            // Brazo en voladizo inclinado a 4.2m (suspendido sin colisión)
            glm::vec3 armPos = poleBase + glm::vec3(0.0f, 4.2f, 0.0f) - sideDir * 0.55f;
            float armRot = isHorizontal ? (sideSign > 0 ? 180.0f : 0.0f) : (sideSign > 0 ? 270.0f : 90.0f);
            addProp("lampArm", armPos, armRot, glm::vec3(1.0f), false);

            // Foco / Luminaria incandescente suspendida sobre la pista (sin colisión)
            glm::vec3 bulbPos = poleBase + glm::vec3(0.0f, 4.1f, 0.0f) - sideDir * 1.15f;
            addProp("lampBulb", bulbPos, 0.0f, glm::vec3(1.0f), false);

            m_lampPositions.push_back(bulbPos);
        }
    }

    // 3. Arcos de Neón Luminosos Exclusivos para Circuitos Nocturnos
    if (isNight && straightTiles.size() >= 6) {
        size_t idx1 = straightTiles.size() / 3;
        size_t idx2 = (straightTiles.size() * 2) / 3;
        const TileData* neonTiles[2] = { straightTiles[idx1], straightTiles[idx2] };
        const char* trims[2] = { "neonArchTrimCyan", "neonArchTrimMagenta" };

        for (int k = 0; k < 2; ++k) {
            const auto* nTile = neonTiles[k];
            bool isH = (nTile->rotationDegrees == 90.0f || nTile->rotationDegrees == 270.0f);
            glm::vec3 sDir = isH ? glm::vec3(0.0f, 0.0f, 1.0f) : glm::vec3(1.0f, 0.0f, 0.0f);
            float bRot = isH ? 90.0f : 0.0f;

            glm::vec3 lp = nTile->worldPos + sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            glm::vec3 rp = nTile->worldPos - sDir * 2.35f + glm::vec3(0.0f, 2.30f, 0.0f);
            addProp("neonArchPillar", lp, 0.0f, glm::vec3(1.0f), true);
            addProp("neonArchPillar", rp, 0.0f, glm::vec3(1.0f), true);

            glm::vec3 bp = nTile->worldPos + glm::vec3(0.0f, 4.60f, 0.0f);
            addProp("neonArchBeam", bp, bRot, glm::vec3(1.0f), false);

            glm::vec3 tp = nTile->worldPos + glm::vec3(0.0f, 4.40f, 0.0f);
            addProp(trims[k], tp, bRot, glm::vec3(1.0f), false);
        }
    }

    // 4. Elementos de Pista (Curvas, Rectas, Meta)
    for (const auto& tile : grid.getAllTiles()) {
        if (tile.type == TileType::EMPTY) continue;

        // Curvas:
        if (tile.type == TileType::ROAD_CORNER_SMALL) {
            float rot = tile.rotationDegrees;
            glm::vec3 cornerPos = tile.worldPos;

            float xSign = (rot == 0.0f || rot == 90.0f) ? 1.0f : -1.0f;
            float zSign = (rot == 0.0f || rot == 270.0f) ? 1.0f : -1.0f;

            if (isNight) {
                // Baliza luminosa en el ápice interior de la curva
                glm::vec3 beaconPos = cornerPos + glm::vec3(-xSign * halfTile * 0.52f, 0.0f, -zSign * halfTile * 0.52f);
                addProp("neonBeacon", beaconPos, 0.0f, glm::vec3(1.0f), true);
                addProp("neonBeaconTop", beaconPos + glm::vec3(0.0f, 0.45f, 0.0f), 0.0f, glm::vec3(1.0f), false);

                // Doble pila de neumáticos de seguridad en la escapatoria exterior
                glm::vec3 tirePos1 = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.25f, zSign * (halfTile * 1.05f));
                glm::vec3 tirePos2 = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.65f, zSign * (halfTile * 1.05f));
                addProp("debris-tire", tirePos1, rot + 45.0f, glm::vec3(1.2f));
                addProp("debris-tire", tirePos2, rot + 45.0f, glm::vec3(1.0f));

                glm::vec3 barPos = cornerPos + glm::vec3(xSign * halfTile * 0.88f, 0.0f, zSign * halfTile * 0.40f);
                addProp("barrierRed", barPos, rot + 30.0f, glm::vec3(1.1f));
            } else {
                // Modo diurno: Barreras bicolor (Rojo/Blanco), neumáticos de seguridad y conos de ápice
                glm::vec3 barrier1Pos = cornerPos + glm::vec3(xSign * halfTile * 0.88f, 0.0f, zSign * halfTile * 0.40f);
                addProp("barrierRed", barrier1Pos, rot + 30.0f, glm::vec3(1.1f));

                glm::vec3 barrier2Pos = cornerPos + glm::vec3(xSign * halfTile * 0.40f, 0.0f, zSign * halfTile * 0.88f);
                addProp("barrierWhite", barrier2Pos, rot + 60.0f, glm::vec3(1.1f));

                glm::vec3 tirePos = cornerPos + glm::vec3(xSign * (halfTile * 1.05f), 0.25f, zSign * (halfTile * 1.05f));
                addProp("debris-tire", tirePos, rot + 45.0f, glm::vec3(1.1f));

                glm::vec3 apexPos = cornerPos + glm::vec3(-xSign * halfTile * 0.52f, 0.0f, -zSign * halfTile * 0.52f);
                addProp("cone", apexPos, 0.0f, glm::vec3(1.0f));
            }
        }

        // Recta de Meta: Muros de boxes y cajas de equipo
        if (tile.type == TileType::START_FINISH) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            glm::vec3 wallOffset1 = isHorizontal ? glm::vec3(0.0f, 0.0f, -halfTile * 0.95f) : glm::vec3(-halfTile * 0.95f, 0.0f, 0.0f);
            glm::vec3 wallOffset2 = isHorizontal ? glm::vec3(0.0f, 0.0f,  halfTile * 0.95f) : glm::vec3( halfTile * 0.95f, 0.0f, 0.0f);

            addProp("barrierWall", tile.worldPos + wallOffset1, tile.rotationDegrees, glm::vec3(1.0f));
            addProp("barrierWall", tile.worldPos + wallOffset2, tile.rotationDegrees, glm::vec3(1.0f));

            glm::vec3 boxPos = tile.worldPos + wallOffset2 + (isHorizontal ? glm::vec3(1.2f, 0.0f, 0.8f) : glm::vec3(0.8f, 0.0f, 1.2f));
            addProp("box", boxPos, 15.0f, glm::vec3(1.0f));
        }

        // Rectas del circuito: Conos y delimitadores a los lados
        if (tile.type == TileType::ROAD_STRAIGHT) {
            bool isHorizontal = (tile.rotationDegrees == 90.0f || tile.rotationDegrees == 270.0f);
            float lateralDist = halfTile * 0.92f;

            if ((tile.col + tile.row) % 3 == 0) {
                float side = ((propIndex++) % 2 == 0) ? 1.0f : -1.0f;
                glm::vec3 propOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, side * lateralDist) : glm::vec3(side * lateralDist, 0.0f, 0.0f);

                if (propIndex % 4 == 0) {
                    addProp("barrierRed", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.0f));
                } else if (propIndex % 4 == 2) {
                    addProp("barrierWhite", tile.worldPos + propOffset, tile.rotationDegrees, glm::vec3(1.0f));
                } else {
                    addProp("cone", tile.worldPos + propOffset, 0.0f, glm::vec3(1.0f));
                }
            }

            if ((tile.col * 7 + tile.row * 13) % 19 == 0) {
                float decorSide = ((tile.col % 2 == 0) ? 1.0f : -1.0f) * (halfTile * 1.45f);
                glm::vec3 decorOffset = isHorizontal ? glm::vec3(0.0f, 0.0f, decorSide) : glm::vec3(decorSide, 0.0f, 0.0f);
                if ((tile.col + tile.row) % 2 == 0) {
                    addProp("box", tile.worldPos + decorOffset, 25.0f, glm::vec3(0.9f));
                } else {
                    addProp("debris-tire", tile.worldPos + decorOffset + glm::vec3(0.0f, 0.25f, 0.0f), 0.0f, glm::vec3(1.0f));
                }
            }
        }
    }

    // 5. Bosques y Pinos Low-Poly en las Áreas Abiertas de Césped (Infield & Outfield)
    int treeMod = isNight ? 6 : 3;
    for (int r = 1; r < grid.getRows() - 1; ++r) {
        for (int c = 1; c < grid.getCols() - 1; ++c) {
            const auto* current = grid.getTile(c, r);
            if (current && current->type == TileType::EMPTY) {
                bool nearRoad = false;
                for (int dr = -1; dr <= 1 && !nearRoad; ++dr) {
                    for (int dc = -1; dc <= 1; ++dc) {
                        const auto* n = grid.getTile(c + dc, r + dr);
                        if (n && n->type != TileType::EMPTY) {
                            nearRoad = true;
                            break;
                        }
                    }
                }

                if (!nearRoad && (c * 13 + r * 17) % treeMod == 0) {
                    glm::vec3 treePos = current->worldPos + glm::vec3(
                        ((c % 3) - 1.0f) * 0.8f,
                        0.0f,
                        ((r % 3) - 1.0f) * 0.8f
                    );
                    float treeRot = static_cast<float>((c * 37 + r * 53) % 360);

                    addProp("treeTrunk", treePos + glm::vec3(0.0f, 0.8f, 0.0f), treeRot, glm::vec3(1.0f), true);
                    addProp("treeFoliageBottom", treePos + glm::vec3(0.0f, 1.85f, 0.0f), treeRot + 15.0f, glm::vec3(1.0f), false);
                    addProp("treeFoliageTop", treePos + glm::vec3(0.0f, 2.70f, 0.0f), treeRot + 30.0f, glm::vec3(1.0f), false);
                }
            }
        }
    }

    // 6. Decoración del Fondo y Entorno de Estadio Perimetral (Torres Floodlight y Muros)
    float halfW = (grid.getCols() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;
    float halfD = (grid.getRows() * TrackGrid::TILE_SIZE) * 0.5f + 7.0f;

    // 4 Torres de Iluminación en las esquinas
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f, -halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f, -halfD), -45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f,  halfD), 45.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f,  halfD), -45.0f, glm::vec3(1.0f));

    // Torres intermedias en los laterales
    addProp("stadiumTower", glm::vec3(0.0f, 5.0f, -halfD), 0.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(0.0f, 5.0f,  halfD), 0.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3(-halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));
    addProp("stadiumTower", glm::vec3( halfW, 5.0f, 0.0f), 90.0f, glm::vec3(1.0f));

    // Vallas de contención de estadio alrededor de la pista
    for (float x = -halfW + 12.0f; x <= halfW - 12.0f; x += 12.0f) {
        addProp("barrierWall", glm::vec3(x, 0.0f, -halfD), 0.0f, glm::vec3(2.5f));
        addProp("barrierWall", glm::vec3(x, 0.0f,  halfD), 0.0f, glm::vec3(2.5f));
    }
    for (float z = -halfD + 12.0f; z <= halfD - 12.0f; z += 12.0f) {
        addProp("barrierWall", glm::vec3(-halfW, 0.0f, z), 90.0f, glm::vec3(2.5f));
        addProp("barrierWall", glm::vec3( halfW, 0.0f, z), 90.0f, glm::vec3(2.5f));
    }

    std::cout << "[OK] [PropSpawner] Total de elementos generados: "
              << m_props.size() << " (Arco monumental, farolas, torres, "
              << (isNight ? "arcos de neon y balizas nocturnas)." : "pinos y defensas diurnas).") << std::endl;
}

void PropSpawner::render(const renderer::Shader& shader) const {
    if (!m_initialized) return;

    for (const auto& prop : m_props) {
        auto it = m_propMeshes.find(prop.type);
        if (it != m_propMeshes.end()) {
            shader.setMat4("u_Model", prop.modelMatrix);

            if (prop.useCustomColor) {
                // Color neón personalizado (estrellas flotantes con brillo no-blanco)
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", prop.customColor);
            } else if (prop.type == "lampBulb") {
                // Luminaria incandescente de vapor de sodio (ámbar brillante)
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.88f, 0.30f, 1.0f));
            } else if (prop.type == "neonArchTrimCyan") {
                // Ribete emisivo neon cian brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
            } else if (prop.type == "neonArchTrimMagenta") {
                // Ribete emisivo neon magenta electrico
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.12f, 0.72f, 1.0f));
            } else if (prop.type == "neonBeaconTop") {
                // Cabezal luminoso ambar/naranja de baliza de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.50f, 0.05f, 1.0f));
            } else if (prop.type == "neonBeacon") {
                // Base de baliza de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.20f, 0.22f, 1.0f));
            } else if (prop.type == "gantryLight") {
                // Luces de salida de carrera rojas
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.18f, 0.18f, 1.0f));
            } else if (prop.type == "gantryFooting") {
                // Zapata de hormigon gris medio
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.48f, 0.50f, 0.54f, 1.0f));
            } else if (prop.type == "neonArchPillar" || prop.type == "neonArchBeam") {
                // Estructura metalica oscura para arcos de neon
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.12f, 0.14f, 0.18f, 1.0f));
            } else if (prop.type == "lampPole" || prop.type == "lampArm" || prop.type == "gantryPillar" || prop.type == "gantryBeam" || prop.type == "gantryLowerBeam") {
                // Estructura metálica industrial gris oscuro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.24f, 0.26f, 0.30f, 1.0f));
            } else if (prop.type == "gantrySign") {
                // Cartel de Meta rojo vibrante de carreras
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.92f, 0.18f, 0.18f, 1.0f));
            } else if (prop.type == "stadiumTower") {
                // Estructura de torre de estadio gris metalizado
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.38f, 0.42f, 0.48f, 1.0f));
            } else if (prop.type == "treeTrunk") {
                // Tronco de madera marrón oscuro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.42f, 0.28f, 0.16f, 1.0f));
            } else if (prop.type == "treeFoliageBottom") {
                // Follaje de pino verde bosque
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.18f, 0.48f, 0.22f, 1.0f));
            } else if (prop.type == "treeFoliageTop") {
                // Follaje de pino verde vivo superior
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.25f, 0.58f, 0.28f, 1.0f));
            } else if (prop.type == "cactusTrunk" || prop.type == "cactusArmJoint" || prop.type == "cactusArmSpire") {
                // Cactus Saguaro verde desierto vivo
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.48f, 0.22f, 1.0f));
            } else if (prop.type == "desertRock" || prop.type == "desertMesaRock") {
                // Formaciones de arenisca y monolitos rojizo/ocre del canon
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.76f, 0.46f, 0.28f, 1.0f));
            } else if (prop.type == "desertArchPillar" || prop.type == "desertArchBeam") {
                // Arco de piedra natural arenisca tallada
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.70f, 0.42f, 0.25f, 1.0f));
            } else if (prop.type == "spaceWarpTrimCyan") {
                // Emisivo cian hiperespacial brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
            } else if (prop.type == "spaceWarpTrimMagenta") {
                // Emisivo magenta hiperespacial brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.15f, 0.85f, 1.0f));
            } else if (prop.type == "spaceWarpPillar" || prop.type == "spaceWarpBeam") {
                // Estructura de aleacion espacial blanca/gris metalizado pulido
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.85f, 0.88f, 0.94f, 1.0f));
            } else if (prop.type == "satellitePanel") {
                // Panel solar azul cobalto fotovoltaico
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.10f, 0.35f, 0.85f, 1.0f));
            } else if (prop.type == "satelliteBody" || prop.type == "satelliteAntenna") {
                // Chasis de satelite dorado termico (manta de mylar oro)
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.95f, 0.80f, 0.25f, 1.0f));
            } else if (prop.type == "spaceBeaconTop") {
                // Baliza estroboscopica cian de curva
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.10f, 0.90f, 1.0f, 1.0f));
            } else if (prop.type == "spaceBeacon") {
                // Base metalica de baliza
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.22f, 0.28f, 1.0f));
            } else if (prop.type == "floatingStar") {
                // Estrella flotante por defecto en cian neón brillante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.0f, 0.95f, 1.0f, 1.0f));
            } else if (prop.type == "coralBranchTall" || prop.type == "coralBranchSide") {
                // Ramas de coral naranja vibrante
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.45f, 0.18f, 1.0f));
            } else if (prop.type == "coralBrain") {
                // Coral cerebro violeta/magenta bioluminiscente
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.85f, 0.22f, 0.65f, 1.0f));
            } else if (prop.type == "kelpSegment") {
                // Algas marinas Kelp verde esmeralda marino
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.08f, 0.68f, 0.38f, 1.0f));
            } else if (prop.type == "clamShell") {
                // Ostra de concha nacarada / marfil marino con destello violáceo
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.85f, 0.82f, 0.88f, 1.0f));
            } else if (prop.type == "bubbleColumn") {
                // Columna de burbujas marinas translúcidas y brillantes
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.60f, 0.94f, 1.0f, 0.75f));
            } else if (prop.type == "seaRock") {
                // Rocas submarinas de basalto oceánico
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.12f, 0.22f, 0.28f, 1.0f));
            } else if (prop.type == "seaArchPillar" || prop.type == "seaArchBeam") {
                // Templo Atlante de piedra marina aguamarina
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.18f, 0.42f, 0.48f, 1.0f));
            } else if (prop.type == "seaArchTrim") {
                // Ribete nacarado de conchas luminiscentes
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.45f, 0.95f, 0.90f, 1.0f));
            } else if (prop.type == "seaPedestal") {
                // Pedestal de piedra marina
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.15f, 0.25f, 0.30f, 1.0f));
            } else if (prop.type == "seaPearl") {
                // Perla marina con brillo turquesa bioluminiscente intenso
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.98f, 0.88f, 1.0f));
            } else if (prop.type == "volcanoArchPillar" || prop.type == "volcanoArchBeam") {
                // Basalto volcánico negro
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.12f, 0.11f, 0.13f, 1.0f));
            } else if (prop.type == "volcanoArchTrimLava") {
                // Vena de magma ardiente naranja incandescente
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.40f, 0.05f, 1.0f));
            } else if (prop.type == "basaltColumn") {
                // Obsidiana/basalto gris carbón
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.16f, 0.15f, 0.18f, 1.0f));
            } else if (prop.type == "volcanoRock") {
                // Roca volcánica porosa con matiz rojizo
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.24f, 0.16f, 0.14f, 1.0f));
            } else if (prop.type == "lavaChimney") {
                // Chimenea de escoria volcánica
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.20f, 0.14f, 0.12f, 1.0f));
            } else if (prop.type == "lavaVent") {
                // Respiradero de magma amarillo-naranja ardiente
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.55f, 0.08f, 1.0f));
            } else if (prop.type == "fireBrazier") {
                // Soporte de hierro forjado volcánico / copa de piedra basáltica oscura
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(0.16f, 0.15f, 0.17f, 1.0f));
            } else if (prop.type == "volcanoFlame" || prop.type == "flameOrb") {
                // Lenguas de fuego ardiente en tono naranja-rojo incandescente
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.38f, 0.04f, 1.0f));
            } else if (prop.type == "volcanoFlameCore") {
                // Núcleo central caliente en tono amarillo brillante intenso
                shader.setBool("u_UseTexture", false);
                shader.setVec4("u_Color", glm::vec4(1.0f, 0.88f, 0.18f, 1.0f));
            } else {
                // Mallas texturizadas con la paleta de Kenney
                shader.setBool("u_UseTexture", true);
                shader.setVec4("u_Color", glm::vec4(1.0f));
            }

            it->second.draw();
        }
    }
}

} // namespace world
