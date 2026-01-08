#version 330 core

// Posicao do vertice no espaco local do modelo
layout(location = 1) in vec3 inPosition;

// Normal do vertice no espaco local
layout(location = 2) in vec3 inNormal;

// Coordenadas de textura (nao usadas neste shader)
layout(location = 3) in vec2 inTexcoord;


out vec3 fragPos; // Posicao do fragmento em espaco de mundo
out vec3 fragNormal; // Normal interpolada em espaco de mundo
out vec2 exTexcoord; // Coordenadas de textura interpoladas


uniform mat4 ModelMatrix;

// Uniform Buffer Object da camara (Contem as matrizes de view e projection)
uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};


void main(void)
{
    // Converter a posicao do vertice de local para world space
    vec4 worldPos = ModelMatrix * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    // Calculo da normal matrix (Necessario para transformar normais corretamente quando existem escalas nao uniformes)
    mat3 normalMatrix = transpose(inverse(mat3(ModelMatrix)));
    fragNormal = normalize(normalMatrix * inNormal);

    // coordenadas de textura sem alteracoes
    exTexcoord = inTexcoord;

    // Posicao final do vertice em clip space
    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
