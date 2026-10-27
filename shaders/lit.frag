#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoords;
in vec4 v_Color;

out vec4 FragColor;

// -------------------------------------------------------------
// Estructuras de Iluminacion Multi-Fuente
// -------------------------------------------------------------

#define MAX_POINT_LIGHTS 8

// 1. Fuente Direccional (Sol del circuito)
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};

// 2. Fuente Puntual (Postes de luz / farolas del circuito)
struct PointLight {
    vec3 position;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

// 3. Fuente Focal / Spotlight (Faros delanteros del kart)
struct SpotLight {
    vec3 position;
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    float cutOff;
    float outerCutOff;
    float constant;
    float linear;
    float quadratic;
    bool enabled;
};

// Material
struct Material {
    float shininess;
};

// -------------------------------------------------------------
// Uniforms
// -------------------------------------------------------------
uniform DirLight u_DirLight;
uniform PointLight u_PointLights[MAX_POINT_LIGHTS];
uniform int u_NumPointLights;
uniform SpotLight u_SpotLight;
uniform Material u_Material;

// Selector de Modelo de Iluminación:
// 0 = Flat Shading, 1 = Lambert Shading, 2 = Blinn-Phong Shading
uniform int u_LightingModel;
uniform vec3 u_ViewPos;

// Muestreo de textura y color
uniform sampler2D u_Texture;
uniform bool u_UseTexture;
uniform bool u_UseVertexColor;
uniform vec4 u_Color;

// Sustitución dinámica de césped nativo de Kenney para armonización de biomas
uniform bool u_ReplaceKenneyGrass;
uniform vec4 u_GrassReplacementColor;

// -------------------------------------------------------------
// Funciones de Cálculo Fotométrico
// -------------------------------------------------------------

// Cálculo de la luz direccional (Sol)
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(-light.direction);

    // Componente difusa (Lambertiana)
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Caso 2: Blinn-Phong Shading (Half-Way Vector)
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.45;
    }

    return ambient + diffuse + specular;
}

// Cálculo de cada luz puntual con atenuación física calculada
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    
    // Atenuación física calculada: F_att = 1.0 / (constant + linear * d + quadratic * d^2)
    float attenuation = 1.0 / max(light.constant + light.linear * dist + light.quadratic * (dist * dist), 0.0001);

    // Componente difusa
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Caso 2: Blinn-Phong Shading
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.50;
    }

    return (ambient + diffuse + specular) * attenuation;
}

// Cálculo de la luz focal del kart con cono y penumbra angular suave
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / max(light.constant + light.linear * dist + light.quadratic * (dist * dist), 0.0001);

    // Desvanecimiento suave en los bordes de la penumbra conica:
    // intensity = clamp((theta - outerCutOff) / (cutOff - outerCutOff), 0.0, 1.0)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = max(light.cutOff - light.outerCutOff, 0.0001);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Caso 2: Blinn-Phong Shading
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.65;
    }

    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

// -------------------------------------------------------------
// Programa Principal
// -------------------------------------------------------------
void main() {
    vec4 texColor;
    if (u_UseTexture) {
        texColor = texture(u_Texture, v_TexCoords);
    } else if (u_UseVertexColor) {
        texColor = v_Color * u_Color;
    } else {
        texColor = u_Color;
    }

    if (u_ReplaceKenneyGrass) {
        vec3 c = u_UseVertexColor ? v_Color.rgb : texColor.rgb;
        if (abs(c.r - 0.30196) < 0.03 &&
            abs(c.g - 0.56078) < 0.03 &&
            abs(c.b - 0.43137) < 0.03) {
            texColor = u_GrassReplacementColor;
        }
    }

    if (texColor.a < 0.05) discard;

    vec3 baseColor = texColor.rgb;
    vec3 normal;

    // Conmutación de modelo de normales:
    if (u_LightingModel == 0) {
        // Flat Shading: Normal facetada calculada por triangulo
        vec3 dX = dFdx(v_FragPos);
        vec3 dY = dFdy(v_FragPos);
        normal = normalize(cross(dX, dY));
        if (dot(normal, v_Normal) < 0.0) {
            normal = -normal;
        }
    } else {
        // Lambert y Blinn-Phong: Normal suave interpolada
        normal = normalize(v_Normal);
    }

    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    // Acumulación simultánea de las 3 fuentes de luz:
    vec3 totalLight = vec3(0.0);

    // 1. Luz Direccional (Sol)
    totalLight += calcDirLight(u_DirLight, normal, viewDir, baseColor, u_LightingModel);

    // 2. Múltiples Luces Puntuales (Farolas de pista)
    int numPoints = min(u_NumPointLights, MAX_POINT_LIGHTS);
    for (int i = 0; i < numPoints; ++i) {
        totalLight += calcPointLight(u_PointLights[i], normal, v_FragPos, viewDir, baseColor, u_LightingModel);
    }

    // 3. Luz Focal (Faros del kart)
    totalLight += calcSpotLight(u_SpotLight, normal, v_FragPos, viewDir, baseColor, u_LightingModel);

    // Respaldo defensivo si ninguna luz está activa
    if (!u_DirLight.enabled && numPoints == 0 && !u_SpotLight.enabled) {
        totalLight = baseColor * 0.35;
    }

    FragColor = vec4(totalLight, texColor.a);
}
