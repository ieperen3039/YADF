#version 130
#pragma optionNV(unroll all)

struct Light {
    vec4 mPosition;
    vec3 color;
    float intensity;
};

struct Material {
    vec4 diffuse;
    vec4 specular;
    float reflectance;
};

const int MAX_NUM_LIGHTS = 16;
const int MAX_NUM_MATERIALS = 16;
const float MIN_LIGHT_INTENSITY = 0.001f;

in vec3 mVertexNormal;
in vec3 mVertexPosition;
flat in int mMaterialBit;

out vec4 fragColor;

uniform Material materials[MAX_NUM_MATERIALS];
uniform Light lights[MAX_NUM_LIGHTS];
uniform vec3 cameraPosition;
uniform vec3 ambientLight;

vec3 calcSurface(Light light, vec3 vNorm, vec3 unitToLight, vec3 eye, Material mat){
    vec3 result = vec3(0.0, 0.0, 0.0);

    // diffuse component
    float intensity = max(0.0, dot(vNorm, unitToLight));
    if (intensity != 0.0) {
        result += intensity * light.color * mat.diffuse.xyz;

        // specular
        vec3 lightReflect = reflect(unitToLight, vNorm);
        vec3 virtualLightPosition = normalize(-lightReflect);
        float linearSpec = max(0.0, dot(virtualLightPosition, normalize(eye)));
        float shine = pow(linearSpec, mat.reflectance);
        result += shine * shine * light.color * mat.specular.xyz * mat.specular.w;
    }

    return result;
}

vec3 pointLightEffect(Light light, vec3 vPos, vec3 vNorm, vec3 eye, Material mat){
    if (light.intensity == 0.0) return vec3(0.0, 0.0, 0.0);

    vec3 vecToLight = light.mPosition.xyz - vPos;
    float distance = length(vecToLight);

    float maxLight = light.intensity / distance;
    if (maxLight < MIN_LIGHT_INTENSITY) return vec3(0.0, 0.0, 0.0);

    vec3 unitToLight = vecToLight / distance;
    return calcSurface(light, vNorm, unitToLight, eye, mat) / distance;
}

vec3 infiniteLightEffect(Light light, vec3 vNorm, vec3 eye, Material mat){
    vec3 unitToLight = normalize(light.mPosition.xyz);
    return calcSurface(light, vNorm, unitToLight, eye, mat);
}

vec3 lightEffect(Light l, vec3 vPos, vec3 vNorm, vec3 eye, Material mat){
    if (l.intensity > 0){
        if (l.mPosition.w == 0){
            return infiniteLightEffect(l, vNorm, eye, mat);
        } else {
            return pointLightEffect(l, vPos, vNorm, eye, mat);
        }
    } else {
        return vec3(0.0, 0.0, 0.0);
    }
}

void main() {
    float camDot = dot(mVertexNormal, normalize(cameraPosition));
    Material mat = materials[mMaterialBit];

    vec3 diffuse = mat.diffuse.xyz * ambientLight * camDot;

    for (int i = 0; i < MAX_NUM_LIGHTS; i++){
        Light l = lights[i];
        diffuse += lightEffect(l, mVertexPosition, mVertexNormal, cameraPosition, mat);
    }

    fragColor = vec4(diffuse, mat.diffuse.a);
}
