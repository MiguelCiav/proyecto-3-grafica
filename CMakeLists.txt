cmake_minimum_required(VERSION 3.20)
project(MarioKartCGI VERSION 1.0.0 LANGUAGES C CXX)

# ==============================================================================
# 1. Configuración del Estándar C++17
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Opciones de compilación generales
if(MSVC)
    add_compile_options(/W4 /permissive-)
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# ==============================================================================
# 2. Gestión de Dependencias (OpenGL, GLFW, GLM)
# ==============================================================================
include(FetchContent)

# 2.1 OpenGL
find_package(OpenGL REQUIRED)

# 2.2 GLFW 3.4 (Descarga y compilación estática portable sin requerir sudo)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
    GIT_SHALLOW TRUE
)

# 2.3 GLM 1.0.1 (Header-only mathematics library)
set(GLM_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLM_BUILD_INSTALL OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
    GIT_SHALLOW TRUE
)

message(STATUS "Descargando y configurando dependencias de GLFW y GLM...")
FetchContent_MakeAvailable(glfw glm)

# 2.4 GLAD (OpenGL 3.3 Core Profile Loader en vendor/)
add_library(glad STATIC
    vendor/glad/src/glad.c
    vendor/glad/include/glad/glad.h
    vendor/glad/include/KHR/khrplatform.h
)
target_include_directories(glad PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
)
if(NOT MSVC)
    target_compile_options(glad PRIVATE -w)
endif()

# ==============================================================================
# 3. Estructura de Inclusiones del Proyecto
# ==============================================================================
set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader
)

# ==============================================================================
# 4. Definición de Fuentes del Motor
# ==============================================================================
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    ${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp
)

# ==============================================================================
# 5. Ejecutable Principal
# ==============================================================================
add_executable(mario_kart ${ENGINE_SOURCES})

target_include_directories(mario_kart PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(mario_kart PRIVATE
    OpenGL::GL
    glfw
    glm::glm
    glad
    ${CMAKE_DL_LIBS}
)

if(MINGW)
    target_link_options(mario_kart PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Copia de shaders y assets al directorio de ejecución
add_custom_command(TARGET mario_kart POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:mario_kart>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:mario_kart>/assets
    COMMENT "Copiando shaders y assets al directorio de salida binaria..."
)

# ==============================================================================
# 6. Ejecutable de Pruebas Unitarias / Preprocesamiento (Fase 2 - Issue #8)
# ==============================================================================
add_executable(test_mesh_preprocessing
    tests/test_mesh_preprocessing.cpp
    src/geometry/MeshProcessor.cpp
    src/geometry/KartMeshExtractor.cpp
    src/geometry/KartPreprocessor.cpp
)

target_include_directories(test_mesh_preprocessing PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_mesh_preprocessing PRIVATE
    glm::glm
)

add_custom_command(TARGET test_mesh_preprocessing POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:test_mesh_preprocessing>/assets
    COMMENT "Copiando assets al directorio de salida de tests..."
)

# ==============================================================================
# 7. Ejecutable de Pruebas Unitarias - Sistema de Iluminación
# ==============================================================================
add_executable(test_light_manager
    tests/test_light_manager.cpp
    src/lighting/LightManager.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_light_manager PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_light_manager PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

# ==============================================================================
# 8. Ejecutable de Pruebas Unitarias - Compilación de Shaders de Iluminación
# ==============================================================================
add_executable(test_shader_lighting
    tests/test_shader_lighting.cpp
    src/lighting/LightManager.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_shader_lighting PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_shader_lighting PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_shader_lighting POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_shader_lighting>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_shader_lighting..."
)

# ==============================================================================
# 9. Ejecutable de Pruebas Unitarias - Controlador de Iluminación
# ==============================================================================
add_executable(test_lighting_controller
    tests/test_lighting_controller.cpp
    src/lighting/LightManager.cpp
    src/lighting/LightingController.cpp
    src/core/Input.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_lighting_controller PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_lighting_controller PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

# ==============================================================================
# 10. Ejecutable de Pruebas Unitarias - Integración de Iluminación en Escena
# ==============================================================================
add_executable(test_scene_lighting_integration
    tests/test_scene_lighting_integration.cpp
    src/lighting/LightManager.cpp
    src/lighting/LightingController.cpp
    src/renderer/Shader.cpp
    src/world/TrackGrid.cpp
    src/world/LevelLoader.cpp
    src/core/Input.cpp
)

target_include_directories(test_scene_lighting_integration PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_scene_lighting_integration PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_scene_lighting_integration POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_scene_lighting_integration>/shaders
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/assets
        $<TARGET_FILE_DIR:test_scene_lighting_integration>/assets
    COMMENT "Copiando shaders y assets al directorio de salida de test_scene_lighting_integration..."
)

# ==============================================================================
# 11. Ejecutable de Pruebas Unitarias - Skybox con Cubemap 3D
# ==============================================================================
add_executable(test_skybox
    tests/test_skybox.cpp
    src/renderer/Skybox.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
)

target_include_directories(test_skybox PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_skybox PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_skybox POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_skybox>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_skybox..."
)

# ==============================================================================
# 12. Ejecutable de Pruebas Unitarias - Shader Procedural de Boosters
# ==============================================================================
add_executable(test_shader_booster_procedural
    tests/test_shader_booster_procedural.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_shader_booster_procedural PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_shader_booster_procedural PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_shader_booster_procedural POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_shader_booster_procedural>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_shader_booster_procedural..."
)

# ==============================================================================
# 13. Ejecutable de Pruebas Unitarias - Entidad BoosterPad
# ==============================================================================
add_executable(test_booster_pad
    tests/test_booster_pad.cpp
    src/world/BoosterPad.cpp
    src/world/ZoneTrigger.cpp
    src/physics/KartPhysics.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_booster_pad PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_booster_pad PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_booster_pad POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_booster_pad>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_booster_pad..."
)

# ==============================================================================
# 14. Ejecutable de Pruebas Unitarias - Integración Skybox y Boosters en Circuito
# ==============================================================================
add_executable(test_skybox_booster_integration
    tests/test_skybox_booster_integration.cpp
    src/renderer/Skybox.cpp
    src/renderer/Texture.cpp
    src/renderer/Shader.cpp
    src/world/BoosterPad.cpp
    src/world/ZoneTrigger.cpp
    src/world/TrackGrid.cpp
    src/world/LevelLoader.cpp
    src/physics/KartPhysics.cpp
)

target_include_directories(test_skybox_booster_integration PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_skybox_booster_integration PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_skybox_booster_integration POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_skybox_booster_integration>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_skybox_booster_integration..."
)

# ==============================================================================
# 15. Ejecutable de Pruebas Unitarias - Sistema de Partículas 3D
# ==============================================================================
add_executable(test_particle_system
    tests/test_particle_system.cpp
    src/vfx/ParticleSystem.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_particle_system PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_particle_system PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_particle_system POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_particle_system>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_particle_system..."
)

# ==============================================================================
# 16. Ejecutable de Pruebas Unitarias - Emisores de Partículas del Kart
# ==============================================================================
add_executable(test_kart_particle_emitter
    tests/test_kart_particle_emitter.cpp
    src/vfx/KartParticleEmitter.cpp
    src/vfx/ParticleSystem.cpp
    src/physics/KartPhysics.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_kart_particle_emitter PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_kart_particle_emitter PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_kart_particle_emitter POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_kart_particle_emitter>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_kart_particle_emitter..."
)

# ==============================================================================
# 17. Ejecutable de Pruebas Unitarias - Integración Final Mario Kart 20/20
# ==============================================================================
add_executable(test_final_integration
    tests/test_final_integration.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
    src/renderer/Skybox.cpp
    src/lighting/LightManager.cpp
    src/lighting/LightingController.cpp
    src/vfx/KartParticleEmitter.cpp
    src/vfx/ParticleSystem.cpp
    src/world/TrackGrid.cpp
    src/world/LevelLoader.cpp
    src/world/BoosterPad.cpp
    src/world/ZoneTrigger.cpp
    src/physics/KartPhysics.cpp
    src/physics/CollisionSystem.cpp
    src/camera/Camera.cpp
    src/camera/ChaseCamera.cpp
    src/core/Input.cpp
)

target_include_directories(test_final_integration PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_final_integration PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_custom_command(TARGET test_final_integration POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${CMAKE_CURRENT_SOURCE_DIR}/shaders
        $<TARGET_FILE_DIR:test_final_integration>/shaders
    COMMENT "Copiando shaders al directorio de salida de test_final_integration..."
)

# ==============================================================================
# 18. Ejecutable de Pruebas: Giro de Ruedas, Skidmarks y HUD de Carreras
# ==============================================================================
add_executable(test_hud_and_skidmarks
    tests/test_hud_and_skidmarks.cpp
    src/vfx/SkidmarkManager.cpp
    src/ui/RacingHud.cpp
    src/animation/WheelAnimator.cpp
    src/physics/KartPhysics.cpp
    src/renderer/Shader.cpp
)

target_include_directories(test_hud_and_skidmarks PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_hud_and_skidmarks PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

# ==============================================================================
# 19. Target de Empaquetado para Entrega UCV
# ==============================================================================
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    add_custom_target(package_project
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/package.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generando paquete ZIP para entrega final UCV..."
    )
endif()











