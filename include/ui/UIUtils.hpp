#pragma once

#include <glm/glm.hpp>
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstddef>

namespace ui {

/**
 * @brief Puntos cardinales y de anclaje para posicionamiento relativo en pantallas 2D.
 */
enum class UIAnchor {
    TOP_LEFT,
    TOP_CENTER,
    TOP_RIGHT,
    CENTER_LEFT,
    CENTER,
    CENTER_RIGHT,
    BOTTOM_LEFT,
    BOTTOM_CENTER,
    BOTTOM_RIGHT
};

/**
 * @brief Calcula la posicion superior-izquierda (x, y) de un elemento de dimensiones (elemW, elemH)
 * anclado respecto a las dimensiones de la pantalla (screenW, screenH) con desplazamientos opcionales.
 *
 * @param anchor Punto de anclaje cardinal relativo a la pantalla.
 * @param elemW Ancho del elemento de interfaz.
 * @param elemH Alto del elemento de interfaz.
 * @param screenW Ancho actual del viewport / pantalla.
 * @param screenH Alto actual del viewport / pantalla.
 * @param offsetX Desplazamiento horizontal adicional.
 * @param offsetY Desplazamiento vertical adicional.
 * @return glm::vec2 Coordenada (x, y) superior-izquierda resultante.
 */
inline glm::vec2 computeAnchoredPosition(UIAnchor anchor, float elemW, float elemH,
                                         float screenW, float screenH,
                                         float offsetX = 0.0f, float offsetY = 0.0f) {
    float x = 0.0f;
    float y = 0.0f;

    switch (anchor) {
        case UIAnchor::TOP_LEFT:
            x = 0.0f;
            y = 0.0f;
            break;
        case UIAnchor::TOP_CENTER:
            x = (screenW - elemW) * 0.5f;
            y = 0.0f;
            break;
        case UIAnchor::TOP_RIGHT:
            x = screenW - elemW;
            y = 0.0f;
            break;
        case UIAnchor::CENTER_LEFT:
            x = 0.0f;
            y = (screenH - elemH) * 0.5f;
            break;
        case UIAnchor::CENTER:
            x = (screenW - elemW) * 0.5f;
            y = (screenH - elemH) * 0.5f;
            break;
        case UIAnchor::CENTER_RIGHT:
            x = screenW - elemW;
            y = (screenH - elemH) * 0.5f;
            break;
        case UIAnchor::BOTTOM_LEFT:
            x = 0.0f;
            y = screenH - elemH;
            break;
        case UIAnchor::BOTTOM_CENTER:
            x = (screenW - elemW) * 0.5f;
            y = screenH - elemH;
            break;
        case UIAnchor::BOTTOM_RIGHT:
            x = screenW - elemW;
            y = screenH - elemH;
            break;
    }

    return glm::vec2(x + offsetX, y + offsetY);
}

/**
 * @brief Helper para resolver rutas relativas de assets verificando si el ejecutable
 * se ejecuta desde la raiz del repositorio o desde la carpeta de construccion build/.
 *
 * @param relativePath Ruta relativa del asset (ej. "assets/kenney_ui-pack/...").
 * @return std::string Ruta resuelta existente o la original si no se encuentra.
 */
inline std::string resolveAssetPath(const std::string& relativePath) {
    std::ifstream f1(relativePath);
    if (f1.good()) return relativePath;

    std::string parentRelative = "../" + relativePath;
    std::ifstream f2(parentRelative);
    if (f2.good()) return parentRelative;

    return relativePath;
}

/**
 * @brief Sanitiza una cadena de texto para su renderizado con la fuente bitmap 5x7 de UIRenderer:
 * normaliza caracteres UTF-8 acentuados a su version ASCII basica en mayusculas,
 * y mapea caracteres no imprimibles a espacios.
 *
 * @param text Cadena de texto de entrada en UTF-8 o ASCII.
 * @return std::string Cadena normalizada en mayusculas compatible con UIRenderer.
 */
inline std::string sanitizeText(const std::string& text) {
    std::string result;
    result.reserve(text.size());

    for (size_t i = 0; i < text.size(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);

        // Manejo de caracteres acentuados comunes en castellano (UTF-8 de 2 bytes: 0xC3)
        if (c == 0xC3 && i + 1 < text.size()) {
            unsigned char c2 = static_cast<unsigned char>(text[++i]);
            switch (c2) {
                case 0xA1: case 0x81: result += 'A'; break; // á, Á
                case 0xA9: case 0x89: result += 'E'; break; // é, É
                case 0xAD: case 0x8D: result += 'I'; break; // í, Í
                case 0xB3: case 0x93: result += 'O'; break; // ó, Ó
                case 0xBA: case 0x9A: result += 'U'; break; // ú, Ú
                case 0xB1: case 0x91: result += 'N'; break; // ñ, Ñ
                case 0xBC: case 0x9C: result += 'U'; break; // ü, Ü
                default: result += '?'; break;
            }
            continue;
        }

        if (c >= 32 && c <= 126) {
            result += static_cast<char>(std::toupper(c));
        } else {
            result += ' ';
        }
    }

    return result;
}

/**
 * @brief Ajusta y envuelve un texto en lineas que no superen maxWidth,
 * asumiendo el ancho estandar por glifo de 6.0f * scale del UIRenderer (5x7 + 1px espaciado).
 *
 * @param text Texto de entrada.
 * @param maxWidth Ancho maximo en pixeles para cada linea.
 * @param scale Escala de renderizado tipografico aplicada.
 * @return std::vector<std::string> Lista de lineas ajustadas.
 */
inline std::vector<std::string> wrapText(const std::string& text, float maxWidth, float scale) {
    std::vector<std::string> lines;
    if (text.empty() || maxWidth <= 0.0f || scale <= 0.0f) return lines;

    std::string clean = sanitizeText(text);
    std::istringstream words(clean);
    std::string word;
    std::string currentLine;

    const float glyphWidth = 6.0f * scale;

    while (words >> word) {
        std::string testLine = currentLine.empty() ? word : currentLine + " " + word;
        float testW = static_cast<float>(testLine.size()) * glyphWidth;
        if (testW <= maxWidth) {
            currentLine = testLine;
        } else {
            if (!currentLine.empty()) {
                lines.push_back(currentLine);
            }
            currentLine = word;
        }
    }
    if (!currentLine.empty()) {
        lines.push_back(currentLine);
    }
    return lines;
}

} // namespace ui
