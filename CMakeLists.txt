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
