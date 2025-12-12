#version 330 core

in vec3 fragPos;
in vec3 fragNormal;

out vec4 FragmentColor;

uniform vec3 baseColor;

// Light
uniform vec3 lightPos;
uniform vec3 lightColor;

// Camera
uniform vec3 viewPos;

// Material
uniform float ambientStrength;
uniform float specularStrength;
uniform float shininess;

void main(void)
{
    vec3 N = normalize(fragNormal);
    vec3 L = normalize(lightPos - fragPos);
    vec3 V = normalize(viewPos - fragPos);
    vec3 H = normalize(L + V); // Blinn-Phong

    // Ambient
    vec3 ambient = ambientStrength * lightColor;

    // Diffuse
    float diff = max(dot(N, L), 0.0);
    vec3 diffuse = diff * lightColor;

    // Specular
    float spec = pow(max(dot(N, H), 0.0), shininess);
    vec3 specular = specularStrength * spec * lightColor;

    vec3 result = (ambient + diffuse) * baseColor + specular;
    FragmentColor = vec4(result, 1.0);
}
