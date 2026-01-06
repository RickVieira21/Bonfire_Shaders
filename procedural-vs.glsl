#version 330 core

// Posicao do vertice no model space
layout (location = 1) in vec3 inPosition;

// Normal do vertice wm model space
layout (location = 2) in vec3 inNormal;

// ------------------- OUT (para o fragment shader) --------------------

// Posicao do vertice em coordenadas do mundo
out vec3 exPosition;

// Normal transformada para coordenadas do mundo
out vec3 exNormal;

// Matriz de transformacao do modelo (escala, rotacao, translacao)
uniform mat4 ModelMatrix;

// Bloco uniforme da camara 
layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main()
{
    // Converter a posicao do vertice de model space para world space
    exPosition = vec3(ModelMatrix * vec4(inPosition, 1.0));

    // Calcular a normal em world space
    exNormal = mat3(transpose(inverse(ModelMatrix))) * inNormal;

    // Posicao final do vertice em clip space
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(exPosition, 1.0);
}
