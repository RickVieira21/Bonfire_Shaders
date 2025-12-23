#version 330 core

in vec2 TexCoord;
in float gLife;

out vec4 FragColor;

void main()
{
    // Vida invertida:  gLife = 0 nasce, gLife = 1 morre
    float life = 1.0 - gLife;

    // chama (interior quente e exterior frio)
    vec3 innerColor = vec3(1.0, 1.0, 0.8);   // quase branco
    vec3 midColor   = vec3(1.0, 0.6, 0.2);   // laranja
    vec3 outerColor = vec3(0.9, 0.1, 0.0);   // vermelho

    vec3 color;
    if (life > 0.6)
        color = mix(midColor, innerColor, (life - 0.6) / 0.4);
    else
        color = mix(outerColor, midColor, life / 0.6);

    // Intensidade emissiva ( aka brilho real)
    float intensity = life * 1.5;

    // Alpha para suavizar o fade
    float alpha = life;

    FragColor = vec4(color * intensity, alpha);
}

