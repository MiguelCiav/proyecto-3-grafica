#include "vehicle/KartEntity.hpp"
#include <iostream>

namespace vehicle {

KartEntity::KartEntity() = default;

bool KartEntity::setup(const geometry::KartAssembly& assembly) {
    // 1. Cargar geometrias a la GPU
    m_chassisMesh.setup(assembly.chassis);

    m_wheelFLMesh.setup(assembly.wheelFrontLeft);
    m_wheelFRMesh.setup(assembly.wheelFrontRight);
    m_wheelBLMesh.setup(assembly.wheelBackLeft);
    m_wheelBRMesh.setup(assembly.wheelBackRight);

    if (!assembly.driver.vertices.empty()) {
        m_driverMesh.setup(assembly.driver);
    }

    // 2. Construir el Grafo de Escena Jerarquico
    // Nodo Raiz: Chasis (Gobierna la traslacion en el mundo y el yaw global)
    // Escala al 48% (0.48x) para una proporcion visual y espacial optima con el ancho de pista
    m_rootNode = std::make_unique<scene::SceneNode>("Kart_Root_Chassis");
    m_rootNode->getTransform().scale = glm::vec3(0.48f);
    m_rootNode->setMesh(&m_chassisMesh);

    // Piloto: Anclado relativo al habitaculo del chasis con offset configurable por vehiculo
    if (m_driverMesh.isLoaded()) {
        m_driverNode = m_rootNode->createChild("Kart_Driver");
        m_driverNode->getTransform().position = assembly.driver.pivotOffset;
        m_driverNode->setMesh(&m_driverMesh);
    }

    // Rueda Delantera Izquierda (FL): En su pivotOffset relativo al chasis
    m_wheelFLNode = m_rootNode->createChild("Wheel_Front_Left");
    m_wheelFLNode->getTransform().position = assembly.wheelFrontLeft.pivotOffset;
    m_wheelFLNode->setMesh(&m_wheelFLMesh);

    // Rueda Delantera Derecha (FR)
    m_wheelFRNode = m_rootNode->createChild("Wheel_Front_Right");
    m_wheelFRNode->getTransform().position = assembly.wheelFrontRight.pivotOffset;
    m_wheelFRNode->setMesh(&m_wheelFRMesh);

    // Rueda Trasera Izquierda (BL)
    m_wheelBLNode = m_rootNode->createChild("Wheel_Back_Left");
    m_wheelBLNode->getTransform().position = assembly.wheelBackLeft.pivotOffset;
    m_wheelBLNode->setMesh(&m_wheelBLMesh);

    // Rueda Trasera Derecha (BR)
    m_wheelBRNode = m_rootNode->createChild("Wheel_Back_Right");
    m_wheelBRNode->getTransform().position = assembly.wheelBackRight.pivotOffset;
    m_wheelBRNode->setMesh(&m_wheelBRMesh);

    m_initialized = true;
    std::cout << "[OK] [KartEntity] Grafo de escena jerarquico del kart ensamblado con exito." << std::endl;
    return true;
}

void KartEntity::update(const glm::vec3& worldPos, float yawDegrees, const animation::WheelAnimator& animator) {
    if (!m_initialized) return;

    // 1. Actualizar posicion y rumbo del Chasis raiz
    m_rootNode->getTransform().position = worldPos;
    m_rootNode->getTransform().rotationEuler.y = yawDegrees;

    // 2. Actualizar rotaciones locales de ruedas delanteras (Roll en X + Steer en Y)
    float roll = animator.getRollAngle();
    // Invertir signo: en coordenadas diestras de OpenGL (-Z adelante, +X derecha),
    // el viraje a la derecha ('D') requiere rotacion negativa en Y para apuntar hacia +X.
    float steer = -animator.getSteerAngle();

    if (m_wheelFLNode) {
        m_wheelFLNode->getTransform().rotationEuler = glm::vec3(roll, steer, 0.0f);
    }
    if (m_wheelFRNode) {
        m_wheelFRNode->getTransform().rotationEuler = glm::vec3(roll, steer, 0.0f);
    }

    // 3. Actualizar ruedas traseras (Solo roll en X, sin viraje)
    if (m_wheelBLNode) {
        m_wheelBLNode->getTransform().rotationEuler = glm::vec3(roll, 0.0f, 0.0f);
    }
    if (m_wheelBRNode) {
        m_wheelBRNode->getTransform().rotationEuler = glm::vec3(roll, 0.0f, 0.0f);
    }

    // 4. Propagacion recursiva de matrices de mundo M_world = M_parent * M_local
    m_rootNode->updateTransform(glm::mat4(1.0f));
}

void KartEntity::draw(const renderer::Shader& shader) const {
    if (!m_initialized) return;
    m_rootNode->draw(shader);
}

} // namespace vehicle
