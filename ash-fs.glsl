#version 330 core

// ==================== INPUTS ====================
// Posição do fragmento em world (vinda do vertex shader)
in vec3 exPosition;

// Normal interpolada em world space
in vec3 exNormal;

// Cor final do fragmento
out vec4 FragColor;


// ==================== UNIFORMS DE ILUMINAÇÃO ====================

uniform vec3 lightPos;

uniform vec3 lightColor;

// Posição da câmara (necessária para especular)
uniform vec3 viewPos;


// ==================== UNIFORMS DE MATERIAL ====================

uniform float ambientStrength;

uniform float specularStrength;

uniform float shininess;

// ==================== FUNÇÕES DE NOISE ====================

// Função hash simples para aleatoriedade a partir da posição (entre 0 e 1) e é determinístico
float hash(vec3 p) {
    return fract(
        sin(dot(p, vec3(127.1, 311.7, 74.7))) * 43758.5453
    );
}

// Noise 3D suave (value noise interpolado)
float noise(vec3 p) {
    vec3 i = floor(p);   // célula inteira - qual o cubo
    vec3 f = fract(p);   // posição dentro da célula - onde dentro do cubo (0 a 1)

    // Suavização cúbica (evita transições)
    f = f * f * (3.0 - 2.0 * f);

    // Cada canto do cubo recebe um valor aleatório fixo
    float n000 = hash(i + vec3(0,0,0));
    float n100 = hash(i + vec3(1,0,0));
    float n010 = hash(i + vec3(0,1,0));
    float n110 = hash(i + vec3(1,1,0));
    float n001 = hash(i + vec3(0,0,1));
    float n101 = hash(i + vec3(1,0,1));
    float n011 = hash(i + vec3(0,1,1));
    float n111 = hash(i + vec3(1,1,1));

    // Interpolação trilinear
    return mix(
        mix(mix(n000, n100, f.x), mix(n010, n110, f.x), f.y),
        mix(mix(n001, n101, f.x), mix(n011, n111, f.x), f.y),
        f.z
    );
}


// ==================== MATERIAL PROCEDURAL: ASH ====================

vec3 proceduralAsh(vec3 pos) {

    // Escala global do padrão procedural ("tamanho" visual do grão)
    vec3 p = pos * 2.0;

    // Ruído de baixa frequência para volumes grandes de cinza acumulada
    float base = noise(p * 0.6);

    // Ruído de alta frequência para grão fino (pó / areia)
    float grain = noise(p * 30.0);

    // Aumentar contraste 
    base  = pow(base,  1.4);
    grain = pow(grain, 2.5);

    // Mix: mais grão, mas ainda se vê base
    float ash = mix(base, grain, 0.65);

    // Escurecer ligeiramente para parecer cinza queimada (+alto -> +luz)
    ash *= 0.85;

    // cores da ash
    vec3 darkAsh  = vec3(0.05, 0.05, 0.06); // cinza escuro
    vec3 midAsh   = vec3(0.65, 0.65, 0.67); // cinza médio
    vec3 lightAsh = vec3(0.92, 0.92, 0.92); // quase branco

    // Mistura progressiva das cores
    vec3 color = mix(darkAsh, midAsh, ash);
    color = mix(color, lightAsh, smoothstep(0.65, 1.0, ash));

    return color;
}


// ==================== MAIN ====================

void main()
{
    // Normal normalizada
    vec3 N = normalize(exNormal);

    // Vetor da luz (fogueira)
    vec3 L = normalize(lightPos - exPosition);

    // Vetor da câmara
    vec3 V = normalize(viewPos - exPosition);

    // Vetor halfway (Blinn-Phong)
    vec3 H = normalize(L + V);


    // ==================== CALOR DA FOGUEIRA ====================

    vec3 fireCenter = vec3(0.0f, -0.3f, 0.0f);

    // Distância horizontal ao centro da fogueira
    float distToFire = distance(exPosition.xz, fireCenter.xz);

    // Heat: 1.0 no centro, 0.0 fora (O segundo parâmetro do smoothstep funciona como fireradius)
    float heat = 1.0 - smoothstep(0.0, 0.9, distToFire);

    // Controla a queda de intensidade 
    heat = pow(heat, 1.5);

    vec3 fireTint = vec3(1.0, 0.35, 0.15);


    // ==================== ILUMINAÇÃO (BLINN-PHONG) ====================

    // Ambiente
    vec3 ambient = ambientStrength * lightColor;

    // Difusa
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    // Especular
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;


    // ==================== COR FINAL ====================

    // Cor procedural base da ash
    vec3 baseColor = proceduralAsh(exPosition);

    // Mistura da ash com a cor quente do fogo
    baseColor = mix(baseColor, fireTint, heat * 6.0);

    // Combinação final de iluminação e material
    vec3 result = (ambient + diffuse + specular) * baseColor;

    FragColor = vec4(result, 1.0);
}
