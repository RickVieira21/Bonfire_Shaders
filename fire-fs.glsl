#version 330 core

in vec2 TexCoord;
in float gLife;

out vec4 FragColor;

void main() {

    float alpha = 1.0 - gLife;

    vec3 fireColor = mix(
        vec3(1.0, 0.1, 0.0),
        vec3(1.0, 0.6, 0.3),
        gLife
    );

    FragColor = vec4(fireColor, alpha);
}
