# Protocolo de Trabajo y Directrices para Agentes de IA (AGENTS.md)

Este documento establece las normas operativas, restricciones técnicas y protocolos de colaboración para cualquier agente de IA o desarrollador que participe en el desarrollo del proyecto **Prototipo Mario Kart - CGI Proyecto #3 (UCV)**.

---

## 1. Misión y Restricciones Técnicas Inviolables

1. **Lenguaje y Gráficos:**
   
   - **Lenguaje:** C++17 estándar.
   
   - **API Gráfica:** Modern OpenGL (Core Profile 3.3+).
   
   - **Bibliotecas Base:** GLFW (ventanas y eventos), GLAD (loader de OpenGL), GLM (matemáticas de vectores y matrices), `tinyobjloader` y `stb_image` (recursos I/O en `vendor/`).
   
   - **Prohibición Estricta:** No se permite el uso de motores gráficos preexistentes (Unity, Unreal, Godot, etc.) ni librerías de físicas externas (Bullet, PhysX, etc.).

2. **Restricción "Cero Blender":**
   
   - **Queda terminantemente prohibido asumir o proponer la edición manual de modelos 3D en software de modelado externo (Blender, Maya, 3ds Max).**
   
   - **Recentrado de Pivotes:** El cálculo del centro geométrico/centroide de las ruedas de los karts para permitir su rotación local en `(0, 0, 0)` debe realizarse **estrictamente por código C++** en tiempo de ejecución o preprocesamiento.
   
   - **Ensamblaje del Circuito:** La pista no es un modelo monolítico; se construye proceduralmente mediante una matriz/Tilemap 2D en C++ acoplando las piezas modulares de Kenney.

3. **Catálogo de Assets Autorizados:**
   
   - Se utilizarán exclusivamente los paquetes de Kenney incluidos en el repositorio:
     - `kenney_car-kit.zip` (Karts `kart-oobi.obj`, `kart-oodi.obj`, ruedas, accesorios y `colormap.png`).
     - `kenney_racing-kit.zip` (Piezas modulares de pista: rectas, curvas, barreras, conos y vallas).

4. **Requisitos de Evaluación de la Cátedra (UCV):**
   
   - Al menos 3 fuentes de iluminación distintas (Directional, Point, Spot).
   
   - Controlador en tiempo real para alternar entre al menos 3 modelos de iluminación (Flat, Lambert, Blinn-Phong).
   
   - Al menos 2 objetos con textura.
   
   - Cámara libre tipo FPS con navegación y conmutación a cámara de seguimiento.
   
   - Sistema de colisiones básico (AABB / Esferas envolventes) que restrinja la cámara FPS y el kart.
   
   - Skybox cúbico 3D inmersivo.
   
   - Sistema de partículas 3D con billboarding (humo de escape y boost).
   
   - **Requisito Adicional Elegido:** Texturizado procedural con animaciones en los Boosters/Turbo Pads.

---

## 2. Protocolo de Ejecución de Tareas (Cómo debe actuar el Agente)

### Regla 1: Lectura Previa del Grafo de Issues

- Antes de iniciar cualquier tarea, el agente debe consultar el Issue correspondiente en GitHub (`#1` al `#36`) y verificar el campo **Dependencias**.
- Si un Issue depende de interfaces definidas en otro Issue, el agente debe verificar la presencia de los archivos de cabecera (`.hpp`) correspondientes antes de codificar.

### Regla 2: Filosofía "Header-First" (Desacoplamiento Estricto)

- Al iniciar un módulo, se redacta primero el archivo de cabecera (`.hpp`) con interfaces limpias, guardas de inclusión (`#pragma once`) y *forward declarations* cuando sea posible.
- **Aislamiento de alcance:** Un agente que implementa un Issue asignado a `Dev B` no debe editar archivos asignados a `Dev A` o `Dev C`, interactuando únicamente a través de las interfaces públicas acordadas.

### Regla 3: Flujo de Ramas en Git

- Ningún cambio se realiza directamente sobre `main`.
- Toda tarea se trabaja en su rama de feature:
  
  ```bash
  git checkout -b feature/issue-<ID>-<nombre-descriptivo>
  ```
- Los commits deben referenciar el Issue correspondiente:
  
  ```bash
  git commit -m "feat(module): descripción clara del cambio (#ID)"
  ```

### Regla 4: Calidad de Código C++ y OpenGL Defensivo

- **Gestión de Memoria (RAII):** Utilizar `std::unique_ptr` y `std::shared_ptr`. Toda clase que cree recursos en GPU (`glGenBuffers`, `glGenVertexArrays`, `glGenTextures`, `glCreateProgram`) debe destruirlos explícitamente en su destructor.
- **Validación de Shaders:** Todo shader compilado debe verificar errores (`GL_COMPILE_STATUS`, `GL_LINK_STATUS`) e imprimir logs detallados en la consola.
- **Independencia de Framerate:** Todo cálculo de física, velocidad, animación de giro de ruedas o movimiento de cámara debe multiplicarse obligatoriamente por `deltaTime`.

### Regla 5: Verificación Obligatoria Antes de Completar

El agente **nunca** debe dar por completado un Issue sin ejecutar la verificación local:

```bash
cmake -B build && cmake --build build
```

- El código debe compilar con **cero errores y cero advertencias críticas**.
- Debe verificarse punto por punto cada elemento del **Checklist de Criterios de Aceptación** del Issue.

---

## 4. Mapa de Fases y Milestones del Proyecto

| Fase / Milestone | Objetivo Clave                                                                                | Issues                                                                                                                                |
|:----------------:|:--------------------------------------------------------------------------------------------- |:-------------------------------------------------------------------------------------------------------------------------------------:|
| **Fase 1**       | Setup, CMake, Contexto OpenGL (GLFW/GLAD), Time & Input, Game Loop.                           | [#1](https://github.com/MiguelCiav/proyecto-3-grafica/issues/1) - [#4](https://github.com/MiguelCiav/proyecto-3-grafica/issues/4)     |
| **Fase 2**       | Assets de Kenney, Separación de mallas OBJ y cálculo de pivotes de ruedas a `(0,0,0)`.        | [#5](https://github.com/MiguelCiav/proyecto-3-grafica/issues/5) - [#8](https://github.com/MiguelCiav/proyecto-3-grafica/issues/8)     |
| **Fase 3**       | Shaders base, Loaders (`tinyobjloader`, `stb_image`), Cámara libre FPS y visor 3D.            | [#9](https://github.com/MiguelCiav/proyecto-3-grafica/issues/9) - [#12](https://github.com/MiguelCiav/proyecto-3-grafica/issues/12)   |
| **Fase 4**       | Generación de pista por Tilemap 2D, renderizado modular, obstáculos y zonas especiales.       | [#13](https://github.com/MiguelCiav/proyecto-3-grafica/issues/13) - [#16](https://github.com/MiguelCiav/proyecto-3-grafica/issues/16) |
| **Fase 5**       | Físicas del kart (arcade), sistema de colisiones (AABB/Esferas) y restricción de cámara FPS.  | [#17](https://github.com/MiguelCiav/proyecto-3-grafica/issues/17) - [#20](https://github.com/MiguelCiav/proyecto-3-grafica/issues/20) |
| **Fase 6**       | Grafo de escena, transformaciones jerárquicas, ensamble del kart y animación de ruedas (X/Y). | [#21](https://github.com/MiguelCiav/proyecto-3-grafica/issues/21) - [#24](https://github.com/MiguelCiav/proyecto-3-grafica/issues/24) |
| **Fase 7**       | 3 Fuentes de luz (Directional, Point, Spot) y conmutador de 3 modelos de sombreado.           | [#25](https://github.com/MiguelCiav/proyecto-3-grafica/issues/25) - [#28](https://github.com/MiguelCiav/proyecto-3-grafica/issues/28) |
| **Fase 8**       | Entorno con Skybox 3D (Cubemap) y texturizado procedural animado en Boosters.                 | [#29](https://github.com/MiguelCiav/proyecto-3-grafica/issues/29) - [#32](https://github.com/MiguelCiav/proyecto-3-grafica/issues/32) |
| **Fase 9**       | Sistema de partículas (humo de escape y turbo), script de empaque UCV y defensa 20/20.        | [#33](https://github.com/MiguelCiav/proyecto-3-grafica/issues/33) - [#36](https://github.com/MiguelCiav/proyecto-3-grafica/issues/36) |

---

## 5. Preparación para la Defensa Oral

El código debe estar documentado con explicaciones claras de las decisiones técnicas adoptadas.
