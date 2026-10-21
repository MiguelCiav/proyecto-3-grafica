#version 330 core

in vec3 v_FragPos;
in vec3 v_Normal;
in vec2 v_TexCoords;

out vec4 FragColor;

// -------------------------------------------------------------
// Estructuras de Iluminación Multi-Fuente (Fase 7)
// -------------------------------------------------------------

// 1. Fuente Direccional (Sol)
struct DirLight {
    vec3 direction;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
    bool enabled;
};

// 2. Fuente Puntual (Farola de pista)
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

// 3. Fuente Focal (Faros del kart)
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

// Propiedades del material
struct Material {
    float shininess;
};

// -------------------------------------------------------------
// Uniforms
// -------------------------------------------------------------
uniform DirLight u_DirLight;
uniform PointLight u_PointLight;
uniform SpotLight u_SpotLight;
uniform Material u_Material;

// Modelo de iluminación: 0 = Flat, 1 = Lambert, 2 = Blinn-Phong
uniform int u_LightingModel;
uniform vec3 u_ViewPos;

// Textura y color base
uniform sampler2D u_Texture;
uniform bool u_UseTexture;
uniform vec4 u_Color;

// -------------------------------------------------------------
// Funciones de Cálculo de Iluminación
// -------------------------------------------------------------

// Cálculo de luz direccional (Sol)
vec3 calcDirLight(DirLight light, vec3 normal, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(-light.direction);

    // Componente difusa (Lambertiana)
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Componente especular Blinn-Phong (usando el vector intermedio Half-Way)
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.45;
    }

    return ambient + diffuse + specular;
}

// Cálculo de luz puntual con atenuación cuadrática (Farola de pista)
vec3 calcPointLight(PointLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / max(light.constant + light.linear * dist + light.quadratic * (dist * dist), 0.0001);

    // Componente difusa
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Especular Blinn-Phong
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.50;
    }

    return (ambient + diffuse + specular) * attenuation;
}

// Cálculo de luz focal con atenuación y penumbra angular suave (Faros del kart)
vec3 calcSpotLight(SpotLight light, vec3 normal, vec3 fragPos, vec3 viewDir, vec3 baseColor, int model) {
    if (!light.enabled) return vec3(0.0);

    vec3 lightDir = normalize(light.position - fragPos);
    float dist = length(light.position - fragPos);
    float attenuation = 1.0 / max(light.constant + light.linear * dist + light.quadratic * (dist * dist), 0.0001);

    // Cálculo del cono focal y atenuación suave en el borde (penumbra)
    float theta = dot(lightDir, normalize(-light.direction));
    float epsilon = max(light.cutOff - light.outerCutOff, 0.0001);
    float intensity = clamp((theta - light.outerCutOff) / epsilon, 0.0, 1.0);

    // Componente difusa
    float diff = max(dot(normal, lightDir), 0.0);

    vec3 ambient = light.ambient * baseColor;
    vec3 diffuse = light.diffuse * diff * baseColor;
    vec3 specular = vec3(0.0);

    // Especular Blinn-Phong
    if (model == 2 && diff > 0.0) {
        vec3 halfwayDir = normalize(lightDir + viewDir);
        float spec = pow(max(dot(normal, halfwayDir), 0.0), max(u_Material.shininess, 1.0));
        specular = light.specular * spec * 0.65;
    }

    return (ambient + (diffuse + specular) * intensity) * attenuation;
}

// -------------------------------------------------------------
// Función Principal
// -------------------------------------------------------------
void main() {
    vec4 texColor = u_UseTexture ? texture(u_Texture, v_TexCoords) : u_Color;
    if (texColor.a < 0.05) discard;

    vec3 baseColor = texColor.rgb;
    vec3 normal;

    if (u_LightingModel == 0) {
        // 1. Flat Shading: Normal facetada calculada por derivadas espaciales de pantalla
        vec3 dX = dFdx(v_FragPos);
        vec3 dY = dFdy(v_FragPos);
        normal = normalize(cross(dX, dY));
    } else {
        // 2. Lambert (1) o Blinn-Phong (2): Normal suave interpolada
        normal = normalize(v_Normal);
    }

    vec3 viewDir = normalize(u_ViewPos - v_FragPos);

    // Acumulación de las 3 fuentes de iluminación distintas
    vec3 totalLight = vec3(0.0);
    totalLight += calcDirLight(u_DirLight, normal, viewDir, baseColor, u_LightingModel);
    totalLight += calcPointLight(u_PointLight, normal, v_FragPos, viewDir, baseColor, u_LightingModel);
    totalLight += calcSpotLight(u_SpotLight, normal, v_FragPos, viewDir, baseColor, u_LightingModel);

    // Respaldo de seguridad defensivo si ninguna luz está activa
    if (!u_DirLight.enabled && !u_PointLight.enabled && !u_SpotLight.enabled) {
        totalLight = baseColor * 0.35;
    }

    FragColor = vec4(totalLight, texColor.a);
}
