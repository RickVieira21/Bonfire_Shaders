#version 330 core

in vec3 exPosition;
in vec3 exNormal;

out vec4 FragColor;

// ----------------- Iluminação -----------------
uniform vec3 lightPos;     // posição do fogo
uniform vec3 lightColor;   // cor do fogo
uniform vec3 viewPos;

// material
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

// influência do fogo
uniform float fireRadius;   // raio de influência do fogo
uniform vec3 fireColor;     // cor quente (laranja/vermelho)

// ----------------- Noise -----------------

float hash(vec3 p) {
    return fract(sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453);
}

float noise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);

    f = f * f * (3.0 - 2.0 * f);

    float n000 = hash(i + vec3(0,0,0));
    float n100 = hash(i + vec3(1,0,0));
    float n010 = hash(i + vec3(0,1,0));
    float n110 = hash(i + vec3(1,1,0));
    float n001 = hash(i + vec3(0,0,1));
    float n101 = hash(i + vec3(1,0,1));
    float n011 = hash(i + vec3(0,1,1));
    float n111 = hash(i + vec3(1,1,1));

    return mix(
        mix(mix(n000, n100, f.x), mix(n010, n110, f.x), f.y),
        mix(mix(n001, n101, f.x), mix(n011, n111, f.x), f.y),
        f.z
    );
}

// ----------------- Material (pedra queimada) -----------------

vec3 proceduralCharredStone(vec3 pos) {

    vec3 p = pos * 2.0;

    float base   = noise(p * 0.8);
    float veins  = noise(p * 3.5);
    float grain  = noise(p * 10.0);

    base  = pow(base, 1.3);
    veins = pow(veins, 2.2);
    grain = pow(grain, 3.0);

    float stone = base;
    stone = mix(stone, veins, 0.5);
    stone = mix(stone, grain, 0.2);

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
    vec3 N = normalize(exNormal);
    vec3 L = normalize(lightPos - exPosition);
    vec3 V = normalize(viewPos - exPosition);
    vec3 H = normalize(L + V);

    // distância ao fogo
    float distToFire = distance(exPosition, lightPos);

    // influência térmica
    float fireInfluence = 1.4 - smoothstep(0.0, fireRadius, distToFire);

    // iluminação baseada na distância
    vec3 ambient  = ambientStrength * lightColor * fireInfluence;
    float diff    = max(dot(N, L), 0.0);
    vec3 diffuse  = diff * lightColor * fireInfluence;

    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor * fireInfluence;

    // cor base do terreno
    vec3 baseColor = proceduralCharredStone(exPosition);

    // tingir terreno com calor do fogo
    vec3 emberTint = fireColor * fireInfluence;
    baseColor = mix(baseColor, emberTint, fireInfluence * 0.6);

    vec3 result = (ambient + diffuse + specular) * baseColor;
    FragColor = vec4(result, 1.0);
}
