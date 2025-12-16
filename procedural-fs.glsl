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

// ----------------- Material -----------------

vec3 proceduralStone(vec3 pos) {
    float n1 = noise(pos * 2.0);
    float n2 = noise(pos * 6.0);
    float n = 0.6 * n1 + 0.4 * n2;

    vec3 dark = vec3(0.08, 0.08, 0.08);
    vec3 light = vec3(0.4, 0.4, 0.4);

    return mix(dark, light, n);
}

// ----------------- Main -----------------

void main()
{
    vec3 N = normalize(exNormal);
    vec3 L = normalize(lightPos - exPosition);
    vec3 V = normalize(viewPos - exPosition);
    vec3 H = normalize(L + V);

    vec3 ambient = ambientStrength * lightColor;

    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 baseColor = proceduralStone(exPosition);
    vec3 result = (ambient + diffuse + specular) * baseColor;

    FragColor = vec4(result, 1.0);
}
