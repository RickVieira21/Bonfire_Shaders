#version 330 core

layout (location = 1) in vec3 inPosition;
layout (location = 2) in vec3 inNormal;

out vec3 exPosition;
out vec3 exNormal;

uniform mat4 ModelMatrix;

layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main()
{
    exPosition = vec3(ModelMatrix * vec4(inPosition, 1.0));
    exNormal = mat3(transpose(inverse(ModelMatrix))) * inNormal;

    gl_Position = ProjectionMatrix * ViewMatrix * vec4(exPosition, 1.0);
}
