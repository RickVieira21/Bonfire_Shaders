#version 330 core

layout (location = 1) in vec3 inPosition;
out vec3 TexCoords;

layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main()
{
    TexCoords = inPosition;

    // remover a translação da view matrix
    mat4 view = mat4(mat3(ViewMatrix));
    vec4 pos = ProjectionMatrix * view * vec4(inPosition, 1.0);

    gl_Position = pos.xyww;
}
