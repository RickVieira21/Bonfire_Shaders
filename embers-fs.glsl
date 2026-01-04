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
//uniform vec3 fireCenter;
//uniform float fireRadius;
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

// ----------------- Main -----------------

void main() {

    vec3 N = normalize(exNormal);
    vec3 L = normalize(lightPos - exPosition);
    vec3 V = normalize(viewPos - exPosition);
    vec3 H = normalize(L + V);


// ---------------- CRACK MASK ----------------

float cracks = noise(exPosition * 8.0);
float grain  = noise(exPosition * 25.0);

float mask = smoothstep(0.5, 0.8, cracks);
mask *= grain;

// ---------------- CORES ----------------

vec3 coalColor  = vec3(0.03, 0.03, 0.03);
vec3 emberColor = vec3(1.0, 0.35, 0.08); // fogo puro

vec3 baseColor = mix(coalColor, emberColor, mask);

// ---------------- ILUMINAÇÃO NORMAL ----------------

vec3 ambient = ambientStrength * lightColor;
float diff = max(dot(N, L), 0.0);
vec3 diffuse = diff * lightColor;

float spec = pow(max(dot(N, H), 0.0), shininess);
vec3 specular = specularStrength * spec * lightColor;

vec3 litColor = (ambient + diffuse + specular) * baseColor;

// ---------------- EMISSÃO + PULSE ----------------

vec3 emissive = emberColor * mask * 1 * 3.0;

float slowPulse = 0.4 + 0.4 * sin(time + exPosition.x * 3.0);
emissive *= slowPulse;

// soma direta (não depende da luz!)
vec3 finalColor = litColor + emissive;

FragColor = vec4(finalColor, 1.0);
}

