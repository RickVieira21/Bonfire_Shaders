#version 330 core

// Coordenadas de textura vindas do geometry shader (billboard)
in vec2 TexCoord;

// Vida da particula (0.0 nasce, 1.0 morre)
in float gLife;

// Cor final do fragmento
out vec4 FragColor;

void main()
{
    // Vida invertida:
    // gLife = 0 -> particula acabou de nascer
    // gLife = 1 -> particula esta a morrer
    float life = 1.0 - gLife;

    // cores da chama (do centro para fora)
    vec3 innerColor = vec3(1.0, 1.0, 0.8);   // quase branco
    vec3 midColor   = vec3(1.0, 0.6, 0.2);   // laranja
    vec3 outerColor = vec3(0.9, 0.1, 0.0);   // vermelho

    vec3 color;

    // Se a particula ainda é nova (mais perto do centro) mistura do laranja para o branco
    if (life > 0.6)
        color = mix(midColor, innerColor, (life - 0.6) / 0.4); //not rly used
    else
    // Se é mais velha (mais no exterior) mistura do vermelho para o laranja
        color = mix(outerColor, midColor, life / 0.6);

     // Intensidade emissiva da chama (quanto maior a vida (mais perto do nascimento), mais brilho)
    float intensity = life * 1.5;

    // Alpha controla o fade da particula (particulas novas sao mais solidas, velhas desaparecem)
    float alpha = life;

    // Cor final: (color * intensity simula emissao de luz, e alpha controla transparencia)
    FragColor = vec4(color * intensity, alpha);
}

