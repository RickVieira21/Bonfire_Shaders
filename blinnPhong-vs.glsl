#version 330 core

layout(location = 1) in vec3 inPosition;
layout(location = 2) in vec3 inNormal;
layout(location = 3) in vec2 inTexcoord;

out vec3 fragPos;
out vec3 fragNormal;
out vec2 exTexcoord;

uniform mat4 ModelMatrix;

uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main(void)
{
    vec4 worldPos = ModelMatrix * vec4(inPosition, 1.0);
    fragPos = worldPos.xyz;

    mat3 normalMatrix = transpose(inverse(mat3(ModelMatrix)));
    fragNormal = normalize(normalMatrix * inNormal);

    exTexcoord = inTexcoord;

    gl_Position = ProjectionMatrix * ViewMatrix * worldPos;
}
