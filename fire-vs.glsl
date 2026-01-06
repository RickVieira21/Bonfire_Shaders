#version 330 core

// Atributo por particula: posicao no mundo
layout (location = 0) in vec3 inPosition;

// Atributo por particula: vida normalizada (0 nasce, 1 morre)
layout (location = 1) in float inLife;

// Valor da vida enviado para o geometry shader
out float vLife;

void main() {

    // A posicao e passada diretamente para o pipeline (Neste caso e aplica View ou Projection aqui, porque o geometry shader vai tratar disso)
    gl_Position = vec4(inPosition, 1.0);

    // Passar a vida da particula para o geometry shader
    vLife = inLife;
}
