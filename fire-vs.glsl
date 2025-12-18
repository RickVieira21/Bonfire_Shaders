#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in float inLife;

out float vLife;

void main() {
    gl_Position = vec4(inPosition, 1.0);
    vLife = inLife;
}
