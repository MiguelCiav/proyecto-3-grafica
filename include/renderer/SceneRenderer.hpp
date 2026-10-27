#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <memory>
#include "renderer/Shader.hpp"

// Forward declarations para desacoplamiento Header-First
namespace world {
    class TrackGrid;
    class PropSpawner;
    class BoosterPad;
    enum class TrackTheme;
}

namespace vehicle {
    class KartEntity;
}

namespace vfx {
    class KartParticleEmitter;
    class SkidmarkManager;
}

namespace lighting {
    class LightManager;
}

namespace renderer {

class TrackRenderer;
class Skybox;
class Texture;
class MeshGL;

/**
 * @brief Descriptor completo de la escena para el pase de renderizado 3D.
 * Contiene todas las referencias de mallas, entidades, efectos y matrices de cámara.
 */
struct RenderSceneDesc {
    glm::mat4 viewMatrix{1.0f};
    glm::mat4 projectionMatrix{1.0f};
    glm::vec3 cameraPosition{0.0f};
    float elapsedTime{0.0f};

    const world::TrackGrid* trackGrid{nullptr};
    const renderer::TrackRenderer* trackRenderer{nullptr};
    const world::PropSpawner* propSpawner{nullptr};
    const vehicle::KartEntity* kartEntity{nullptr};
    const std::vector<world::BoosterPad>* boosterPads{nullptr};
    const vfx::KartParticleEmitter* kartEmitter{nullptr};
    vfx::SkidmarkManager* skidmarkManager{nullptr};
    const renderer::Skybox* skybox{nullptr};
    const renderer::Texture* paletteTexture{nullptr};
    const renderer::MeshGL* pedestalMesh{nullptr}; // Para Showroom o decoraciones
    glm::mat4 pedestalTransform{1.0f};
    glm::vec4 pedestalColor{0.20f, 0.22f, 0.28f, 1.0f};

    bool isNight{false};
    bool renderGround{true};
    bool renderProps{true};
    bool renderKart{true};
    bool renderBoosters{true};
    bool renderParticles{true};
    bool renderSkidmarks{true};
    bool renderSkybox{true};
};

/**
 * @brief Orquestador central del pipeline de renderizado 3D (Forward Rendering).
 * Unifica el bucle de renderizado 3D reduciendo la duplicación en escenas y
 * gestiona de forma determinista los 4 pases de dibujo:
 * 1. Geometría Opaca (m_litShader: pista, props, pedestal, kart).
 * 2. Boosters Procedurales (m_boosterShader: flechas animadas y emisión).
 * 3. Efectos Translúcidos (blending: marcas de derrape y partículas).
 * 4. Fondo Inmersivo (Skybox 3D cúbico).
 */
class SceneRenderer {
public:
    SceneRenderer();
    ~SceneRenderer();

    // Regla de 5 RAII: Prohibir copia, permitir movimiento noexcept
    SceneRenderer(const SceneRenderer&) = delete;
    SceneRenderer& operator=(const SceneRenderer&) = delete;

    SceneRenderer(SceneRenderer&& other) noexcept;
    SceneRenderer& operator=(SceneRenderer&& other) noexcept;

    /**
     * @brief Inicializa los shaders internos cargándolos desde el directorio indicado.
     */
    bool init(const std::string& shadersDir = "shaders");

    /**
     * @brief Inicializa el renderer referenciando shaders ya cargados externamente.
     */
    bool init(Shader* litShader, Shader* boosterShader, Texture* paletteTexture = nullptr);

    /**
     * @brief Prepara el framebuffer para un nuevo cuadro con color de borrado explícito.
     */
    void beginFrame(float screenWidth, float screenHeight, const glm::vec4& clearColor);

    /**
     * @brief Prepara el framebuffer calculando automáticamente el color de borrado por bioma.
     */
    void beginFrame(world::TrackTheme theme, bool isNight, float screenWidth, float screenHeight);

    /**
     * @brief Ejecuta la secuencia ordenada de los 4 pases de renderizado 3D.
     */
    void renderScene(const RenderSceneDesc& desc, const lighting::LightManager& lightManager);
    void renderScene(const RenderSceneDesc& desc, const lighting::LightManager* lightManager = nullptr);

    /**
     * @brief Concluye el renderizado 3D y restaura el estado de GPU para la capa 2D/HUD.
     */
    void endFrame();

    /**
     * @brief Calcula el color de borrado (glClearColor) según el bioma y ciclo día/noche.
     */
    static glm::vec4 calculateClearColor(world::TrackTheme theme, bool isNight);

    [[nodiscard]] bool isInitialized() const { return m_initialized; }
    [[nodiscard]] Shader* getLitShader() { return m_litShader; }
    [[nodiscard]] const Shader* getLitShader() const { return m_litShader; }
    [[nodiscard]] Shader* getBoosterShader() { return m_boosterShader; }
    [[nodiscard]] const Shader* getBoosterShader() const { return m_boosterShader; }

private:
    Shader m_ownedLitShader;
    Shader m_ownedBoosterShader;

    Shader* m_litShader{nullptr};
    Shader* m_boosterShader{nullptr};
    Texture* m_paletteTexture{nullptr};

    bool m_initialized{false};
};

} // namespace renderer
