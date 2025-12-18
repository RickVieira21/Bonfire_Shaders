#version 330 core

layout (points) in;
layout (triangle_strip, max_vertices = 4) out;

in float vLife[];
out vec2 TexCoord;
out float gLife;

layout(std140) uniform Camera {
    mat4 ViewMatrix;
    mat4 ProjectionMatrix;
};

void main() {
    vec3 pos = gl_in[0].gl_Position.xyz;

    float size = mix(0.2, 0.05, vLife[0]);

    vec3 right = vec3(ViewMatrix[0][0], ViewMatrix[1][0], ViewMatrix[2][0]) * size;
    vec3 up    = vec3(ViewMatrix[0][1], ViewMatrix[1][1], ViewMatrix[2][1]) * size;

    vec3 p;

    gLife = vLife[0];

    p = pos - right - up;
    TexCoord = vec2(0, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos + right - up;
    TexCoord = vec2(1, 0);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos - right + up;
    TexCoord = vec2(0, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    p = pos + right + up;
    TexCoord = vec2(1, 1);
    gl_Position = ProjectionMatrix * ViewMatrix * vec4(p, 1);
    EmitVertex();

    EndPrimitive();
}
