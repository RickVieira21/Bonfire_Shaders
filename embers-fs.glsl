#version 330 core

in vec3 exPosition;
in vec3 exNormal;

out vec4 FragColor;

// iluminação
uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 viewPos;

// material
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

// fogo
uniform vec3 fireCenter;
uniform float fireRadius;
uniform float time;

// ----------------- Noise -----------------

float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1,311.7,74.7))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    f = f * f * (3.0 - 2.0 * f);

    return mix(
        mix(hash(i), hash(i + vec3(1,0,0)), f.x),
        mix(hash(i + vec3(0,1,0)), hash(i + vec3(1,1,0)), f.x),
        f.y
    );
}

// ----------------- Ember Material -----------------

vec3 proceduralEmber(vec3 pos, float heat) {

    float cracks = noise(pos * 6.0);
    float cracks2 = noise(pos * 12.0);
    float grain = noise(pos * 40.0);

    float veins = abs(cracks - cracks2);
    veins = smoothstep(0.01, 0.15, veins);

    float sparks = smoothstep(0.85, 1.0, grain);

    float mask = max(veins, sparks);
    mask = pow(mask, 0.4);

    vec3 coal  = vec3(0.03);
    vec3 ember = vec3(1.2, 0.5, 0.15);

    return mix(coal, ember, mask * heat);
}


// ----------------- Main -----------------

void main() {

    vec3 N = normalize(exNormal);
    vec3 L = normalize(lightPos - exPosition);
    vec3 V = normalize(viewPos - exPosition);
    vec3 H = normalize(L + V);

    // distância ao fogo
    float dist = distance(exPosition.xz, fireCenter.xz);
    float heat = 1.0 - smoothstep(0.0, fireRadius * 0.8, dist);
    heat = pow(heat, 1.8);

    // flicker térmico
    heat *= 0.7 + 0.3 * sin(time * 3.0 + exPosition.x * 5.0);

    vec3 baseColor = proceduralEmber(exPosition, heat);

    // iluminação clássica
    vec3 ambient = ambientStrength * lightColor;
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    // emissão 
    vec3 emissive = vec3(1.2, 0.5, 0.15) * heat * 3.0;


    vec3 result = (ambient + diffuse + specular) * baseColor + emissive;

    FragColor = vec4(result, 1.0);
}
