#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float vLife[];
out vec2 TexCoord;
out float gLife;

uniform float time;

layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {

    //FLICKER
    float t = time * 2.5;
    float rnd = hash(float(gl_PrimitiveID) * 13.37);

    float heightFactor = clamp(vLife[0], 0.0, 1.0);
    float flickerStrength = heightFactor * 0.06;

    float swayX = sin(t + rnd * 6.2831) * flickerStrength;
    float swayZ = cos(t * 0.7 + rnd * 6.2831) * flickerStrength;

    vec3 pos = gl_in[0].gl_Position.xyz + vec3(swayX, 0.0, swayZ); //POSIÇÃO + FLICKER
    //FIM FLICKER

    float size = mix(0.1, 0.001, vLife[0]); //TAMANHO PARTICULAS

    vec3 right = vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]) * size;
    vec3 up    = vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]) * size;

    gLife = vLife[0];

    vec3 p;

    p = pos - right - up;
    TexCoord = vec2(0, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos + right - up;
    TexCoord = vec2(1, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos - right + up;
    TexCoord = vec2(0, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos + right + up;
    TexCoord = vec2(1, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    EndPrimitive();
}
