#include "geometry/ProceduralGeometry.hpp"
#include <cmath>
#include <vector>
#include <glm/gtc/matrix_transform.hpp>

namespace geometry {

static constexpr float PI = 3.14159265358979323846f;

SubMesh ProceduralGeometry::createBox(float hx, float hy, float hz, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

    // 24 vértices para 6 caras con normales ortogonales precisas y devanado CCW
    // Cara +Y (Superior)
    mesh.vertices.push_back({ {-hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy,  hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx,  hy, -hz}, {0.0f, 1.0f, 0.0f}, {0.0f, 1.0f} });
    // Cara -Y (Inferior)
    mesh.vertices.push_back({ {-hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy, -hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 0.0f} });
    mesh.vertices.push_back({ { hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {1.0f, 1.0f} });
    mesh.vertices.push_back({ {-hx, -hy,  hz}, {0.0f, -1.0f, 0.0f}, {0.0f, 1.0f} });
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

SubMesh ProceduralGeometry::createStar(float outerRadius, float innerRadius, float depth, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

    const int NUM_POINTS = 5;
    std::vector<glm::vec3> perimeter(NUM_POINTS * 2);

    for (int i = 0; i < NUM_POINTS; ++i) {
        float angleTip = (PI * 0.5f) + (static_cast<float>(i) * 2.0f * PI / static_cast<float>(NUM_POINTS));
        perimeter[i * 2] = glm::vec3(outerRadius * std::cos(angleTip), outerRadius * std::sin(angleTip), 0.0f);

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

    for (int i = 0; i < NUM_POINTS * 2; ++i) {
        int next = (i + 1) % (NUM_POINTS * 2);
        addTriangle(frontApex, perimeter[i], perimeter[next]);
        addTriangle(backApex, perimeter[next], perimeter[i]);
    }

    return mesh;
}

SubMesh ProceduralGeometry::createFacetedDome(float radius, float height, int rings, int segments, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

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

    for (int s = 0; s < segments; ++s) {
        int next = (s + 1) % segments;
        addTri(apex, ringVerts[0][next], ringVerts[0][s]);
    }

    for (int r = 0; r < rings - 1; ++r) {
        for (int s = 0; s < segments; ++s) {
            int next = (s + 1) % segments;
            addTri(ringVerts[r][s], ringVerts[r+1][s], ringVerts[r+1][next]);
            addTri(ringVerts[r][s], ringVerts[r+1][next], ringVerts[r][next]);
        }
    }

    glm::vec3 bottomCenter(0.0f, 0.0f, 0.0f);
    int last = rings - 1;
    for (int s = 0; s < segments; ++s) {
        int next = (s + 1) % segments;
        addTri(bottomCenter, ringVerts[last][next], ringVerts[last][s]);
    }

    return mesh;
}

SubMesh ProceduralGeometry::createBrazier(float baseRadius, float bowlRadius, float totalHeight, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;
    const int segments = 8;

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
            addTri(ringVerts[r][s], ringVerts[r][nxt], ringVerts[r+1][nxt]);
            addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r+1][s]);
        }
    }

    glm::vec3 bottomCenter(0.0f, -totalHeight * 0.50f, 0.0f);
    for (int s = 0; s < segments; ++s) {
        int nxt = (s + 1) % segments;
        addTri(bottomCenter, ringVerts[0][s], ringVerts[0][nxt]);
    }

    glm::vec3 innerCoalCenter(0.0f, totalHeight * 0.35f, 0.0f);
    for (int s = 0; s < segments; ++s) {
        int nxt = (s + 1) % segments;
        addTri(innerCoalCenter, ringVerts.back()[nxt], ringVerts.back()[s]);
    }

    return mesh;
}

SubMesh ProceduralGeometry::createFlame(float radius, float height, bool isCore) {
    SubMesh mesh;
    mesh.name = isCore ? "Procedural_Flame_Core" : "Procedural_Volcano_Flame";

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
                addTri(ringVerts[r][s], ringVerts[r][nxt], ringVerts[r+1][nxt]);
                addTri(ringVerts[r][s], ringVerts[r+1][nxt], ringVerts[r+1][s]);
            }
        }

        glm::vec3 apex(0.0f, height, 0.0f);
        size_t last = rings.size() - 1;
        for (int s = 0; s < segments; ++s) {
            int nxt = (s + 1) % segments;
            addTri(ringVerts[last][nxt], apex, ringVerts[last][s]);
        }

        glm::vec3 bottom(0.0f, 0.0f, 0.0f);
        for (int s = 0; s < segments; ++s) {
            int nxt = (s + 1) % segments;
            addTri(bottom, ringVerts[0][s], ringVerts[0][nxt]);
        }
    } else {
        const int numTongues = 5;
        for (int t = 0; t < numTongues; ++t) {
            float ang = static_cast<float>(t) * (2.0f * PI / static_cast<float>(numTongues));
            float thHeight = height * (0.75f + 0.20f * std::sin(static_cast<float>(t) * 1.8f));

            glm::vec3 bL(radius * 0.40f * std::cos(ang - 0.35f), 0.0f, radius * 0.40f * std::sin(ang - 0.35f));
            glm::vec3 bR(radius * 0.40f * std::cos(ang + 0.35f), 0.0f, radius * 0.40f * std::sin(ang + 0.35f));
            glm::vec3 mL(radius * 0.95f * std::cos(ang - 0.20f + 0.15f), thHeight * 0.45f, radius * 0.95f * std::sin(ang - 0.20f + 0.15f));
            glm::vec3 mR(radius * 0.95f * std::cos(ang + 0.20f + 0.15f), thHeight * 0.45f, radius * 0.95f * std::sin(ang + 0.20f + 0.15f));
            glm::vec3 tip(radius * 0.25f * std::cos(ang + 0.35f), thHeight, radius * 0.25f * std::sin(ang + 0.35f));

            addTri(bL, mL, mR);
            addTri(bL, mR, bR);
            addTri(mL, tip, mR);

            addTri(mL, bL, mR);
            addTri(mR, bL, bR);
            addTri(tip, mL, mR);
        }
    }

    return mesh;
}

SubMesh ProceduralGeometry::createSphere(float radius, int rings, int sectors, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

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
        addTri(northPole, ringVerts[0][nxt], ringVerts[0][s]);
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

SubMesh ProceduralGeometry::createClamShell(float radius, float openAngleDeg, const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

    auto addTri = [&](const glm::vec3& v0, const glm::vec3& v1, const glm::vec3& v2) {
        glm::vec3 norm = glm::normalize(glm::cross(v1 - v0, v2 - v0));
        if (glm::length(norm) < 0.001f) return;
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

    const int uSegs = 14;
    const int vSegs = 5;
    const float thickness = 0.035f;
    const float dishDepth = 0.14f;
    const float hingeZ = -radius * 0.40f;

    for (int valve = 0; valve < 2; ++valve) {
        bool isUpper = (valve == 1);
        float tiltRad = glm::radians(isUpper ? openAngleDeg : 0.0f);
        float cosT = std::cos(tiltRad);
        float sinT = std::sin(tiltRad);

        std::vector<std::vector<glm::vec3>> gridOut;
        std::vector<std::vector<glm::vec3>> gridIn;

        for (int vI = 0; vI <= vSegs; ++vI) {
            float v = static_cast<float>(vI) / static_cast<float>(vSegs);
            std::vector<glm::vec3> rowOut, rowIn;

            for (int uI = 0; uI <= uSegs; ++uI) {
                float u = static_cast<float>(uI) / static_cast<float>(uSegs) * PI;
                float fanAngle = u - PI * 0.5f;

                float ribFactor = 1.0f + 0.075f * std::cos(fanAngle * 8.0f);
                float dist = (0.12f + 0.88f * v) * radius * ribFactor;
                float lx = dist * std::sin(fanAngle) * 1.18f;
                float lz = hingeZ + dist * std::cos(fanAngle) * 1.32f;

                float cup = std::sin(v * PI * 0.85f) * std::cos(fanAngle);
                float lyOut = isUpper ? (dishDepth * cup + thickness) : (-dishDepth * cup - thickness);
                float lyIn  = isUpper ? (dishDepth * cup) : (-dishDepth * cup);

                if (isUpper) {
                    float zRel = lz - hingeZ;
                    float yRotOut = lyOut * cosT + zRel * sinT;
                    float zRotOut = -lyOut * sinT + zRel * cosT + hingeZ;

                    float yRotIn  = lyIn * cosT + zRel * sinT;
                    float zRotIn  = -lyIn * sinT + zRel * cosT + hingeZ;

                    rowOut.push_back(glm::vec3(lx, yRotOut + 0.02f, zRotOut));
                    rowIn.push_back(glm::vec3(lx, yRotIn + 0.02f, zRotIn));
                } else {
                    rowOut.push_back(glm::vec3(lx, lyOut, lz));
                    rowIn.push_back(glm::vec3(lx, lyIn, lz));
                }
            }
            gridOut.push_back(rowOut);
            gridIn.push_back(rowIn);
        }

        for (int vI = 0; vI < vSegs; ++vI) {
            for (int uI = 0; uI < uSegs; ++uI) {
                if (!isUpper) {
                    addTri(gridOut[vI][uI], gridOut[vI+1][uI], gridOut[vI+1][uI+1]);
                    addTri(gridOut[vI][uI], gridOut[vI+1][uI+1], gridOut[vI][uI+1]);
                } else {
                    addTri(gridOut[vI][uI], gridOut[vI+1][uI+1], gridOut[vI+1][uI]);
                    addTri(gridOut[vI][uI], gridOut[vI][uI+1], gridOut[vI+1][uI+1]);
                }
            }
        }

        for (int vI = 0; vI < vSegs; ++vI) {
            for (int uI = 0; uI < uSegs; ++uI) {
                if (!isUpper) {
                    addTri(gridIn[vI][uI], gridIn[vI+1][uI+1], gridIn[vI+1][uI]);
                    addTri(gridIn[vI][uI], gridIn[vI][uI+1], gridIn[vI+1][uI+1]);
                } else {
                    addTri(gridIn[vI][uI], gridIn[vI+1][uI], gridIn[vI+1][uI+1]);
                    addTri(gridIn[vI][uI], gridIn[vI+1][uI+1], gridIn[vI][uI+1]);
                }
            }
        }

        for (int uI = 0; uI < uSegs; ++uI) {
            if (!isUpper) {
                addTri(gridOut[vSegs][uI], gridIn[vSegs][uI], gridIn[vSegs][uI+1]);
                addTri(gridOut[vSegs][uI], gridIn[vSegs][uI+1], gridOut[vSegs][uI+1]);
            } else {
                addTri(gridOut[vSegs][uI], gridIn[vSegs][uI+1], gridIn[vSegs][uI]);
                addTri(gridOut[vSegs][uI], gridOut[vSegs][uI+1], gridIn[vSegs][uI+1]);
            }
        }

        for (int vI = 0; vI < vSegs; ++vI) {
            if (!isUpper) {
                addTri(gridOut[vI][0], gridOut[vI+1][0], gridIn[vI+1][0]);
                addTri(gridOut[vI][0], gridIn[vI+1][0], gridIn[vI][0]);
                addTri(gridOut[vI][uSegs], gridIn[vI+1][uSegs], gridOut[vI+1][uSegs]);
                addTri(gridOut[vI][uSegs], gridIn[vI][uSegs], gridIn[vI+1][uSegs]);
            } else {
                addTri(gridOut[vI][0], gridIn[vI+1][0], gridOut[vI+1][0]);
                addTri(gridOut[vI][0], gridIn[vI][0], gridIn[vI+1][0]);
                addTri(gridOut[vI][uSegs], gridOut[vI+1][uSegs], gridIn[vI+1][uSegs]);
                addTri(gridOut[vI][uSegs], gridIn[vI+1][uSegs], gridIn[vI][uSegs]);
            }
        }
    }

    return mesh;
}

SubMesh ProceduralGeometry::createBubbleColumn(const std::string& name) {
    SubMesh mesh;
    mesh.name = name;

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
            addTri(northPole, ringVerts[0][nxt], ringVerts[0][s]);
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

} // namespace geometry
