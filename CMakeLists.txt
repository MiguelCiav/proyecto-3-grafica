cmake_minimum_required(VERSION 3.20)
project(MarioKartCGI VERSION 1.0.0 LANGUAGES C CXX)

# ==============================================================================
# 1. Configuración del Estándar C++17
# ==============================================================================
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Definiciones globales para Windows (evitar macros min/max que rompen std::min, std::max y std::clamp)
if(WIN32)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN)
endif()

# Opciones de compilación generales
if(MSVC)
    add_compile_options(/W4 /permissive- /FS)
    add_compile_definitions(NOMINMAX WIN32_LEAN_AND_MEAN _CRT_SECURE_NO_WARNINGS)
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

# 2.5 miniaudio + stb_vorbis (Audio Engine & Ogg Vorbis Decoder en vendor/miniaudio/)
add_library(miniaudio STATIC
    vendor/miniaudio/miniaudio_impl.c
)
target_include_directories(miniaudio PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/miniaudio
)
if(NOT MSVC)
    target_compile_options(miniaudio PRIVATE -w)
endif()
if(UNIX AND NOT APPLE)
    target_link_libraries(miniaudio PUBLIC pthread m ${CMAKE_DL_LIBS})
endif()

# ==============================================================================
# 3. Estructura de Inclusiones del Proyecto
# ==============================================================================
set(PROJECT_INCLUDE_DIRS
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/glad/include
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/tinyobjloader
    ${CMAKE_CURRENT_SOURCE_DIR}/vendor/miniaudio
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
    miniaudio
    ${CMAKE_DL_LIBS}
)

if(MINGW)
    target_link_options(mario_kart PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# Sincronización Centralizada de Recursos (Shaders y Assets)
add_custom_target(copy_resources ALL
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/shaders"
        "${CMAKE_BINARY_DIR}/shaders"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/assets"
        "${CMAKE_BINARY_DIR}/assets"
    COMMENT "Sincronizando shaders y assets al directorio de salida binaria..."
)
add_dependencies(mario_kart copy_resources)

# ==============================================================================
# 6. Ejecutable de Pruebas Unitarias / Preprocesamiento (Fase 2 - Issue #8)
# ==============================================================================
add_executable(test_mesh_preprocessing
    tests/test_mesh_preprocessing.cpp
    src/geometry/OBJLoader.cpp
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

add_dependencies(test_mesh_preprocessing copy_resources)

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

add_dependencies(test_light_manager copy_resources)

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

add_dependencies(test_shader_lighting copy_resources)

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

add_dependencies(test_lighting_controller copy_resources)

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

add_dependencies(test_scene_lighting_integration copy_resources)

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

add_dependencies(test_skybox copy_resources)

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

add_dependencies(test_shader_booster_procedural copy_resources)

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

add_dependencies(test_booster_pad copy_resources)

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

add_dependencies(test_skybox_booster_integration copy_resources)

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

add_dependencies(test_particle_system copy_resources)

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

add_dependencies(test_kart_particle_emitter copy_resources)

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
    src/physics/CollisionSolver.cpp
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

add_dependencies(test_final_integration copy_resources)

# ==============================================================================
# 18. Ejecutable de Pruebas: Giro de Ruedas, Skidmarks y HUD de Carreras
# ==============================================================================
add_executable(test_hud_and_skidmarks
    tests/test_hud_and_skidmarks.cpp
    src/vfx/SkidmarkManager.cpp
    src/ui/RacingHud.cpp
    src/ui/UIRenderer.cpp
    src/animation/WheelAnimator.cpp
    src/physics/KartPhysics.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
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

add_dependencies(test_hud_and_skidmarks copy_resources)

# ==============================================================================
# 19. Ejecutable de Pruebas: Renderizador 2D de UI por Lotes (UIRenderer)
# ==============================================================================
add_executable(test_ui_renderer
    tests/test_ui_renderer.cpp
    src/ui/UIRenderer.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
)

target_include_directories(test_ui_renderer PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_ui_renderer PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    ${CMAKE_DL_LIBS}
)

add_dependencies(test_ui_renderer copy_resources)

# ==============================================================================
# 20. Ejecutable de Pruebas: Menu Principal y Configuracion GUI (MainMenuUI)
# ==============================================================================
add_executable(test_main_menu_ui
    tests/test_main_menu_ui.cpp
    src/ui/MainMenuUI.cpp
    src/ui/UIWidgets.cpp
    src/ui/UIRenderer.cpp
    src/audio/AudioManager.cpp
    src/renderer/Shader.cpp
    src/renderer/Texture.cpp
    src/lighting/LightManager.cpp
    src/lighting/LightingController.cpp
    src/core/Input.cpp
    src/core/SceneManager.cpp
)

target_include_directories(test_main_menu_ui PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_main_menu_ui PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    miniaudio
    ${CMAKE_DL_LIBS}
)

add_dependencies(test_main_menu_ui copy_resources)

# ==============================================================================
# 21. Ejecutable de Pruebas: Showroom UI y Selección de Piloto/Auto/Pista (ShowroomUI)
# ==============================================================================
set(SHOWROOM_TEST_SOURCES ${ENGINE_SOURCES})
list(REMOVE_ITEM SHOWROOM_TEST_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp")

add_executable(test_showroom_ui
    tests/test_showroom_ui.cpp
    ${SHOWROOM_TEST_SOURCES}
)

target_include_directories(test_showroom_ui PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_showroom_ui PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    miniaudio
    ${CMAKE_DL_LIBS}
)

add_dependencies(test_showroom_ui copy_resources)

# ==============================================================================
# 22. Ejecutable de Pruebas: Spawner de Props y Biomas (PropSpawner)
# ==============================================================================
add_executable(test_prop_spawner
    tests/test_prop_spawner.cpp
    ${SHOWROOM_TEST_SOURCES}
)

target_include_directories(test_prop_spawner PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_prop_spawner PRIVATE
    OpenGL::GL
    glad
    glfw
    glm::glm
    miniaudio
    ${CMAKE_DL_LIBS}
)

add_dependencies(test_prop_spawner copy_resources)

# ==============================================================================
# 23. Ejecutable de Pruebas: Subsistema de Audio (AudioManager + Kenney UI SFX)
# ==============================================================================
add_executable(test_audio_system
    tests/test_audio_system.cpp
    src/audio/AudioManager.cpp
)

target_include_directories(test_audio_system PUBLIC
    ${PROJECT_INCLUDE_DIRS}
)

target_link_libraries(test_audio_system PRIVATE
    glm::glm
    miniaudio
    ${CMAKE_DL_LIBS}
)

add_dependencies(test_audio_system copy_resources)

# ==============================================================================
# 24. Target de Empaquetado para Entrega UCV
# ==============================================================================
find_package(Python3 QUIET COMPONENTS Interpreter)
if(Python3_FOUND)
    add_custom_target(package_project
        COMMAND ${Python3_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/scripts/package.py
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMENT "Generando paquete ZIP para entrega final UCV..."
    )
endif()











