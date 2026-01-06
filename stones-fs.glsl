#version 330 core

in vec3 exPosition;
in vec3 exNormal;

out vec4 FragColor;

// ----------------- Iluminacao -----------------

uniform vec3 lightPos;
uniform vec3 lightColor;

// Posicao da camara (para calculo especular)
uniform vec3 viewPos;

// ----------------- Material -----------------

uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

// ----------------- Noise -----------------

// Funcao hash simples para gerar aleatoriedade
float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

// Noise 3D interpolado (value noise)
float noise(vec3 p) {
    vec3 i = floor(p);   // parte inteira
    vec3 f = fract(p);   // parte fraccionaria

    // Curva suave para interpolacao
    f = f * f * (3.0 - 2.0 * f);

    // Valores nos cantos do cubo
    float n000 = hash(i + vec3(0,0,0));
    float n100 = hash(i + vec3(1,0,0));
    float n010 = hash(i + vec3(0,1,0));
    float n110 = hash(i + vec3(1,1,0));
    float n001 = hash(i + vec3(0,0,1));
    float n101 = hash(i + vec3(1,0,1));
    float n011 = hash(i + vec3(0,1,1));
    float n111 = hash(i + vec3(1,1,1));

    // Interpolacao trilinear
    return mix(
        mix(mix(n000, n100, f.x), mix(n010, n110, f.x), f.y),
        mix(mix(n001, n101, f.x), mix(n011, n111, f.x), f.y),
        f.z
    );
}

// ----------------- Material Procedural -----------------


vec3 proceduralCharredStone(vec3 pos) {

    // Escala do noise
    vec3 p = pos * 200.0;

    // Estrutura base da pedra
    float base = noise(p * 0.8);

    // Veios escuros (fracturas queimadas)
    float veins = noise(p * 300.5);

    // Rugosidade fina
    float grain = noise(p * 10.0);

    // Aumentar contraste
    base = pow(base, 1.3);
    veins = pow(veins, 2.2);
    grain = pow(grain, 3.0);

    float stone = base;
    stone = mix(stone, veins, 0.0);
    stone = mix(stone, grain, 0.2);

    // cores queimadas
    vec3 deepBlack = vec3(0.03, 0.03, 0.03);
    vec3 charcoal  = vec3(0.12, 0.12, 0.12);
    vec3 ashGray   = vec3(0.35, 0.35, 0.35);

    vec3 color = mix(deepBlack, charcoal, stone);
    color = mix(color, ashGray, smoothstep(0.7, 1.0, stone));

    return color;
}

// ----------------- Main -----------------

void main()
{
    // Normal normalizada
    vec3 N = normalize(exNormal);

    // Direccao da luz
    vec3 L = normalize(lightPos - exPosition);

    // Direccao da camara
    vec3 V = normalize(viewPos - exPosition);

    // halfway vector (Blinn-Phong)
    vec3 H = normalize(L + V);

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 baseColor = proceduralCharredStone(exPosition);

    vec3 result = (ambient + diffuse + specular) * baseColor;

    FragColor = vec4(result, 1.0);
}
