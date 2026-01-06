#version 330 core

// Recebe pontos do vertex shader (uma particula = um ponto)
layout (points) in;

// Emite um quad (billboard) por particula
layout (triangle_strip, max_vertices = 4) out;

// Vida da particula vinda do vertex shader
in float vLife[];


// --------------------- OUT -----------------------


// Coordenadas de textura para o fragment shader
out vec2 TexCoord;

// Vida passada para o fragment shader
out float gLife;

// Tempo global para flicker
uniform float time;

// Bloco de camera (view e projection)
layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

// Funcao simples de hash para gerar aleatoriedade
float hash(float n) {
    return fract(sin(n) * 43758.5453);
}

void main() {

    // -------------------- FLICKER --------------------

    // Tempo acelerado para tornar o flicker mais dinamico
    float t = time * 2.5;

    // Valor aleatorio por particula (baseado no ID)
    float rnd = hash(float(gl_PrimitiveID) * 13.37);

    // Fator baseado na vida: particulas mais altas (mais velhas) oscilam mais
    float heightFactor = clamp(vLife[0], 0.0, 1.0);
    float flickerStrength = heightFactor * 0.06;

    // Sway - Oscilacao lateral da chama
    float swayX = sin(t + rnd * 6.2831) * flickerStrength;
    float swayZ = cos(t * 0.7 + rnd * 6.2831) * flickerStrength;

    // Posicao final da particula 
    vec3 pos = gl_in[0].gl_Position.xyz + vec3(swayX, 0.0, swayZ);

    // -------------------- TAMANHO --------------------

    // Tamanho do billboard: grande no inicio, pequeno no fim da vida
    float size = mix(0.3, 0.001, vLife[0]);

    // Vetores right e up da camera para billboarding
    vec3 right = vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]) * size;
    vec3 up    = vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]) * size;

    // Passar vida para o fragment shader
    gLife = vLife[0];

    vec3 p;

    // -------------------- QUAD (BILLBOARD) --------------------

    // Vertice inferior esquerdo
    p = pos - right - up;
    TexCoord = vec2(0, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    // Vertice inferior direito
    p = pos + right - up;
    TexCoord = vec2(1, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    // Vertice superior esquerdo
    p = pos - right + up;
    TexCoord = vec2(0, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    // Vertice superior direito
    p = pos + right + up;
    TexCoord = vec2(1, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    // Termina o primitive (um quad por particula)
    EndPrimitive();
}
