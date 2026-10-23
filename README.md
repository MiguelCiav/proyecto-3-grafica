# Mario Kart 3D - Proyecto #3: Implementación Avanzada
**Universidad Central de Venezuela (UCV)**  
**Facultad de Ciencias - Escuela de Computación**  
**Licenciatura en Computación**  
**Introducción a la Computación Gráfica (Septiembre 2026)**  

---

## 👥 Integrantes del Equipo
- **Miguel Ciavato** - CI: `[CÉDULA]` (`miguelciavato@gmail.com`)
- **César Carios** - CI: `[CÉDULA]` (`cesarcariosbrancovic@gmail.com`)
- **Carlos Cao** - CI: `[CÉDULA]` (`cao345511@gmail.com`)

---

## 🚀 Requisitos del Sistema
- **Compilador C++**: Soporte completo para **C++17** (GCC 9+, Clang 10+, o MSVC 2019+).
- **Sistema de Construcción**: **CMake >= 3.16** y generador (Make o Ninja).
- **API Gráfica**: **OpenGL 3.3 Core Profile** con soporte para GLSL 330.
- **Librerías incluidas en `vendor/` o gestionadas por CMake**:
  - `GLFW 3.3+` (Creación de ventanas y gestión de contexto OpenGL/entradas).
  - `GLAD` (Cargador de punteros a funciones de OpenGL 3.3 Core Profile).
  - `GLM 1.0+` (Matemáticas vectoriales, transformaciones afines y proyecciones).
  - `stb_image` (Carga y descompresión de imágenes PNG).
  - `tinyobjloader` (Importación de geometría 3D Wavefront OBJ).

---

## 🛠️ Instrucciones de Compilación y Ejecución

### En Linux / macOS:
```bash
# 1. Configurar el proyecto con CMake
cmake -B build -DCMAKE_BUILD_TYPE=Release

# 2. Compilar el ejecutable principal y las suites de prueba
cmake --build build -j$(nproc)

# 3. Ejecutar el videojuego
./build/mario_kart
```

### En Windows (PowerShell con MSVC / Ninja):
```powershell
# 1. Configurar el proyecto
cmake -B build -G Ninja -DCMAKE_BUILD_TYPE=Release

# 2. Compilar
cmake --build build

# 3. Ejecutar
.\build\mario_kart.exe
```

---

## 🎮 Mapeo de Controles del Videojuego

### 🏎️ Conducción del Kart (Modo Carrera):
| Tecla | Acción |
| :--- | :--- |
| `W` / `Flecha Arriba` | Acelerar longitudinalmente |
| `S` / `Flecha Abajo` | Frenar / Marcha atrás |
| `A` / `Flecha Izquierda` | Girar a la izquierda |
| `D` / `Flecha Derecha` | Girar a la derecha |
| `R` | Reiniciar el kart a la posición de salida |
| `M` | Volver al menú principal / selector de circuito |
| `ESC` | Salir del juego |

### 🎥 Control de Cámaras:
| Tecla | Acción |
| :--- | :--- |
| `C` | **Alternar modo de cámara**: Cámara Chase (3ra persona cinemática) vs Cámara FPS (Inspección libre) |
| *Ratón* (en modo FPS) | Orientación de la vista (Pitch y Yaw) |
| `W`/`A`/`S`/`D` (FPS) | Desplazamiento tridimensional por la escena |
| `Espacio` / `Shift` (FPS) | Elevar / Descender cámara libre |

### 💡 Iluminación Dinámica y Faros:
| Tecla | Acción |
| :--- | :--- |
| `1` | Activar modelo de iluminación **Blinn-Phong** (Especular suave de microfacetas) |
| `2` | Activar modelo de iluminación **Lambert** (Reflexión difusa cosenoidal pura) |
| `3` | Activar modelo de iluminación **Gouraud / Flat** (Sombreado facetado/por vértice) |
| `L` | Encender / Apagar los **Faros delanteros móviles** del kart (`SpotLight`) |

### 🏁 Menú y Showroom 3D:
| Tecla | Acción |
| :--- | :--- |
| `1` / `2` / `3` | Seleccionar categoría a modificar: `[1] Piloto`, `[2] Vehículo`, `[3] Pista` |
| `Flecha Izquierda` / `Flecha Derecha` | Navegar opciones disponibles dentro de la categoría |
| `Enter` / `Espacio` | Iniciar la carrera en el circuito seleccionado |

---

## 📋 Cumplimiento de la Pauta UCV

### 1. Requisitos Mínimos (100% Implementados)
1. **Iluminación Multi-Fuente (Al menos 3 tipos distintos)**:
   - `DirectionalLight`: Luz solar direccional cenital con componente ambiental, difusa y especular.
   - `PointLight`: Múltiples farolas y balizas de iluminación omnidireccional con atenuación cuadrática $1 / (k_c + k_l d + k_q d^2)$ situadas en los tramos del circuito.
   - `SpotLight`: Dos faros delanteros móviles montados en el kart que siguen dinámicamente la posición y el vector de avance del vehículo con cono interior de máxima intensidad y cono exterior con atenuación hermite (`smoothstep`).
2. **Texturizado con al menos 2 objetos**:
   - Chasis y piloto mapeados mediante atlas de texturas UV (`colormap.png`).
   - Skybox inmersivo con 6 caras cúbicas de alta resolución (`GL_TEXTURE_CUBE_MAP`).
   - Trazado de pista y props con materiales texturizados.
3. **Controlador interactivo de modelo de iluminación**:
   - Conmutación en tiempo real entre **Blinn-Phong** (`1`), **Lambert** (`2`) y **Gouraud/Flat** (`3`) en todos los shaders de la escena sin detener la simulación.
4. **Sistema de Partículas 3D**:
   - Subsistema de partículas con pool prealocado en memoria, actualización física con gravedad y resistencia del aire, y desvanecimiento progresivo del canal alfa ($\alpha = \alpha_0 \cdot \frac{t_{restante}}{t_{vida}}$).
   - Humo de escape reactivo a la aceleración y ráfaga de chispas de turbo (**Boost Burst**) al entrar en contacto con pads de aceleración.
   - Billboarding esférico instanciado en GPU y renderizado translúcido con `glDepthMask(GL_FALSE)` para evitar oclusión negra.
5. **Cámara libre tipo FPS**:
   - Modo de inspección libre con control de mouse y teclado que incluye resolución de colisiones contra las barreras y modelos del escenario.
6. **Skybox 3D**:
   - Renderizado al fondo de la escena utilizando un cubo geométrico proyectado a profundidad máxima ($z = 1.0$) mediante `gl_Position = pos.xyww;`, eliminación de traslación con `mat4(mat3(view))` y función de profundidad `glDepthFunc(GL_LEQUAL)`.
7. **Sistema de Colisiones Básico**:
   - Restricción de movimiento tanto de la cámara FPS como del kart contra barreras, conos y bordes de la pista mediante esferas envolventes (`BoundingSphere`) y cajas orientadas/AABB (`BoundingBox`).

---

### 2. Requisitos Adicionales (Defensa Técnica Oral)

#### ⭐ Requisito Elegido: Texturizado Procedural con Animaciones en Tiempo Real
- **Ubicación en el código**:
  - Shader de vértices: `shaders/booster_procedural.vert`
  - Shader de fragmentos: `shaders/booster_procedural.frag`
  - Clase entidad: `include/world/BoosterPad.hpp` / `src/world/BoosterPad.cpp`
  - Integración: `src/core/Application.cpp` (`setupBoosterPads()`)
- **Formulación Matemática**:
  1. *Flujo continuo de avance*: Se proyectan las coordenadas de textura UV locales en un campo escalar continuo modulado por el tiempo global:
     $$\text{flow} = UV.y \cdot \text{repetitions} - t \cdot \text{speed}$$
     $$v_{anim} = \text{fract}(\text{flow})$$
  2. *Geometría analítica de las flechas Chevron*: Para cada coordenada horizontal simétrica al centro ($arrowDist = |UV.x - 0.5| \cdot 2.0$), la base de la flecha se define como:
     $$\text{chevronBase} = (1.0 - arrowDist) \cdot \text{slope}$$
     La punta de la flecha se adelanta hacia el sentido de marcha ($UV.y$ mayor), formando una cuña `^` que guía la trayectoria de la carrera.
  3. *Antialiasing analítico*: Se evalúa la distancia al borde de la flecha con una transición suave:
     $$\text{arrowMask} = \text{smoothstep}(0.0, \Delta, v_{anim} - \text{chevronBase}) \cdot \text{smoothstep}(\Delta, 0.0, v_{anim} - \text{chevronBase} - \text{thickness})$$
  4. *Modulación de energía y núcleo incandescente*:
     $$\text{pulse} = 0.85 + 0.15 \cdot \sin(t \cdot \text{pulseSpeed})$$
     $$\text{colorFinal} = \text{mix}(\text{baseColor}, \text{energyColor} \cdot \text{emissionIntensity} \cdot \text{pulse}, \text{arrowMask}) + \text{coreColor} \cdot \text{coreMask}$$
- **Defensa ante la cátedra (Por qué no requirió software externo como Blender)**:
  - Todo el patrón visual, las flechas direccionales, la animación de desplazamiento y el pulso de neón son generados **100% analíticamente en GLSL** dentro del fragment shader.
  - La geometría del pad es un cuadrilátero procedural unitario generado puramente en C++ y elevado con un offset milimétrico ($Y = 0.085\text{m}$) sobre el asfalto para eliminar el fenómeno de *z-fighting*.
  - No se emplearon texturas de mapa de bits (PNG, JPG) ni herramientas de modelado/animación externas, satisfaciendo estrictamente la política "Zero Blender" del proyecto y optimizando al máximo el uso de ancho de banda de la GPU.

#### ⭐ Requisitos Adicionales Complementarios Implementados:
- **Billboarding (Simulación de tridimensionalidad)**: Extracción en el vertex shader de los vectores de la base de la cámara ($R$ y $U$) directamente desde la matriz de vista para orientar perpendicularmente los cuadriláteros de partículas hacia el observador en cualquier ángulo.
- **Cámara basada en física (Chase Camera)**: Cámara cinemática en tercera persona con seguimiento elástico mediante amortiguación crítica (spring-damper) que reacciona a los cambios de velocidad y viraje del kart.
- **Sistema de Colisiones con Rebote**: Respuesta de impulso físico y rebote elástico proporcional al ángulo de incidencia frontal en impactos contra barreras.

---

## 📦 Empaquetado y Entrega para la Cátedra UCV

De acuerdo con las condiciones generales de la cátedra, el código fuente debe entregarse en un archivo comprimido con el formato exacto `PROY3_CEDULA1_CEDULA2_CEDULA3.zip` enviado a `bryansilva.dev@gmail.com`.

Para automatizar este proceso y garantizar que no se incluyan archivos temporales, binarios ni el historial de Git, se proporcionan dos scripts de empaquetado:

### Opción 1: Mediante Script Python (Multiplataforma)
```bash
# Uso: python scripts/package.py <CEDULA1> <CEDULA2> <CEDULA3>
python scripts/package.py 12345678 87654321 11223344
```

### Opción 2: Mediante Script Bash (Linux / macOS)
```bash
# Uso: ./scripts/package.sh <CEDULA1> <CEDULA2> <CEDULA3>
chmod +x scripts/package.sh
./scripts/package.sh 12345678 87654321 11223344
```

Ambos scripts empaquetan exclusivamente:
- `src/` (Código fuente C++)
- `include/` (Archivos de cabecera C++)
- `shaders/` (Código fuente GLSL de shaders)
- `assets/` (Modelos OBJ, texturas y niveles de circuito)
- `vendor/` (Dependencias integradas GLAD, STB, TinyOBJLoader)
- `scripts/` (Scripts de automatización y empaquetado)
- `tests/` (Suites de pruebas unitarias automatizadas)
- `CMakeLists.txt`
- `README.md`

Y excluyen estrictamente: `.git/`, `build/`, `.cache/`, `.vs/`, `.vscode/`, ejecutables `.exe`, archivos objeto `.obj` y archivos `.zip`.
