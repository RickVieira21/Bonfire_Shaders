#include "Vertex.h"
#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Shape {
public:
    GLuint VAO, VBO[2];
    GLsizei indexCount;

  //Cada base shape é um conjunto de buffers que permite desenhar cada peça independentemente 
    Shape(const Vertex* vertices, size_t vertSize, const GLubyte* indices, size_t indSize) {
        indexCount = indSize;

        glGenVertexArrays(1, &VAO);
        glGenBuffers(2, VBO);

        glBindVertexArray(VAO);

        glBindBuffer(GL_ARRAY_BUFFER, VBO[0]);
        glBufferData(GL_ARRAY_BUFFER, vertSize, vertices, GL_STATIC_DRAW);

        glEnableVertexAttribArray(0); // POSITION
        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)0);

        glEnableVertexAttribArray(1); // COLOR
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex), (GLvoid*)sizeof(vertices[0].XYZW));

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VBO[1]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indSize, indices, GL_STATIC_DRAW);

        glBindVertexArray(0);
    }

    void draw(GLint matrixId, const glm::mat4& model) {
        glUniformMatrix4fv(matrixId, 1, GL_FALSE, glm::value_ptr(model));
        glBindVertexArray(VAO);
        glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_BYTE, nullptr);
        glBindVertexArray(0);
    }
};
