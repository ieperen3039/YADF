#version 130
//#pragma optionNV(unroll all)

struct Light {
    vec4 mPosition;
    vec3 color;
    float intensity;
};

const int MAX_NUM_LIGHTS = 16;
const int MAX_NUM_MATERIALS = 16;
const float MIN_LIGHT_INTENSITY = 0.001f;

in vec3 mVertexNormal;
in vec3 mVertexPosition;
flat in int mMaterialBit;

out vec4 fragColor;

uniform vec3 colors[MAX_NUM_MATERIALS];
uniform Light lights[MAX_NUM_LIGHTS];
uniform vec3 cameraPosition;
uniform vec3 ambientLight;

vec3 calcSurface(Light light, vec3 vNorm, vec3 unitToLight, vec3 eye, vec3 mat_color){
    vec3 result = vec3(0.0, 0.0, 0.0);

    // diffuse component
    float intensity = dot(vNorm, unitToLight);
    if (intensity > 0.0) {
        result += intensity * light.color * mat_color;
    }

    return result;
}

vec3 pointLightEffect(Light light, vec3 vPos, vec3 vNorm, vec3 eye, vec3 mat_color){
    vec3 vecToLight = light.mPosition.xyz - vPos;
    float distance = length(vecToLight);

    float maxLight = light.intensity / distance;
    if (maxLight < MIN_LIGHT_INTENSITY) return vec3(0.0, 0.0, 0.0);

    vec3 unitToLight = vecToLight / distance;
    return calcSurface(light, vNorm, unitToLight, eye, mat_color) / distance;
}

vec3 infiniteLightEffect(Light light, vec3 vNorm, vec3 eye, vec3 mat_color){
    vec3 unitToLight = normalize(light.mPosition.xyz);
    return calcSurface(light, vNorm, unitToLight, eye, mat_color);
}

vec3 lightEffect(Light l, vec3 vPos, vec3 vNorm, vec3 eye, vec3 mat_color){
    if (l.intensity > 0){
        if (l.mPosition.w == 0){
            return infiniteLightEffect(l, vNorm, eye, mat_color);
        } else {
            return pointLightEffect(l, vPos, vNorm, eye, mat_color);
        }
    } else {
        return vec3(0.0, 0.0, 0.0);
    }
}

void main() {
    float camDot = dot(mVertexNormal, normalize(cameraPosition));
    vec3 color = colors[mMaterialBit];

    vec3 diffuse = color * ambientLight * camDot;

    for (int i = 0; i < MAX_NUM_LIGHTS; i++){
        diffuse += lightEffect(lights[i], mVertexPosition, mVertexNormal, cameraPosition, color);
    }

    fragColor = vec4(diffuse, 1.0);
}
