////////////////////////////////////////////////////////////////////////////////
//
// Basic Triangle rendering
//
// This program demonstrates fundamental Modern OpenGL concepts by rendering
// two triangle instances directly in Clip Space. It serves as an introductory
// example for understanding the OpenGL graphics pipeline and basic shader
// programming.
//
// Key Concepts Demonstrated:
// - Vertex Array Objects (VAOs) and Vertex Buffer Objects (VBOs)
// - Shader program creation and management
// - Attribute and uniform handling
// - Basic transformation matrices
// - Clip space rendering without model/view/projection matrices
//
// Copyright (c) 2013-25 by Carlos Martinho
//
////////////////////////////////////////////////////////////////////////////////

#include <memory>

#include "../mgl/mgl.hpp"
#include "Vertex.h"
#include "shape.cpp"
#include <iostream>


Shape* baseTriangle;
Shape* baseSquare;
Shape* baseParallelogram;


////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
public:
    MyApp() = default;
    ~MyApp() override = default;

    void initCallback(GLFWwindow* win) override;
    void displayCallback(GLFWwindow* win, double elapsed) override;
    void windowCloseCallback(GLFWwindow* win) override;
    void windowSizeCallback(GLFWwindow* win, int width, int height) override;

private:
    const GLuint POSITION = 0, COLOR = 1;
    GLuint VaoId, VboId[2];
    std::unique_ptr<mgl::ShaderProgram> Shaders = nullptr;
    GLint MatrixId;
    GLint ColorId;

    void createShaderProgram();
    //void createBufferObjects();
    void destroyBufferObjects();
    void drawScene();
};

//////////////////////////////////////////////////////////////////////// SHADERs

void MyApp::createShaderProgram() {
    Shaders = std::make_unique<mgl::ShaderProgram>();
    Shaders->addShader(GL_VERTEX_SHADER, "clip-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "clip-fs.glsl");

    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, POSITION);
    Shaders->addAttribute(mgl::COLOR_ATTRIBUTE, COLOR);
    Shaders->addUniform("Matrix");

    // uniform para cor 
    Shaders->addUniform("uColor");

    Shaders->create();

    MatrixId = Shaders->Uniforms["Matrix"].index;

    // guardar o índice do uniform de cor 
    ColorId = Shaders->Uniforms["uColor"].index;

    //para teste
    /*
    std::cout << "Uniforms in shader:" << std::endl;
    for (auto& u : Shaders->Uniforms) {
        std::cout << "  " << u.first << " : index = " << u.second.index << std::endl;
    }
    */

}

//////////////////////////////////////////////////////////////////// VAOs & VBOs

// Base Right Triangle (unitário 2D)             //Tudo branco
const Vertex RightTriangleVertices[] = {
    {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // bottom-left
    {{1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},  // bottom-right
    {{0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}   // top-left
};
const GLubyte RightTriangleIndices[] = { 0, 1, 2 };
 
// Square (2 triângulos)
const Vertex SquareVertices[] = {               //Tudo branco       
    {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{1.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
};
const GLubyte SquareIndices[] = { 0, 1, 2, 0, 2, 3 };

// Parallelogram (2 triângulos)
const Vertex ParallelogramVertices[] = {        //Tudo branco
    {{0.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{1.0f, 0.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{0.0f, 1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}},
    {{1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 1.0f, 1.0f, 1.0f}}
};
const GLubyte ParallelogramIndices[] = { 0, 1, 2, 0, 3, 1};

/*
void MyApp::createBufferObjects() {
    glGenVertexArrays(1, &VaoId);
    glBindVertexArray(VaoId);
    {
        glGenBuffers(2, VboId);

        glBindBuffer(GL_ARRAY_BUFFER, VboId[0]);
        {
            glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);
            glEnableVertexAttribArray(POSITION);
            glVertexAttribPointer(POSITION, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<GLvoid*>(0));
            glEnableVertexAttribArray(COLOR);
            glVertexAttribPointer(
                COLOR, 4, GL_FLOAT, GL_FALSE, sizeof(Vertex),
                reinterpret_cast<GLvoid*>(sizeof(Vertices[0].XYZW)));
        }
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, VboId[1]);
        {
            glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices,
                GL_STATIC_DRAW);
        }
    }
    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    glDeleteBuffers(2, VboId);
}

*/
void MyApp::destroyBufferObjects() {
    glBindVertexArray(VaoId);
    glDisableVertexAttribArray(POSITION);
    glDisableVertexAttribArray(COLOR);
    glDeleteVertexArrays(1, &VaoId);
    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////// SCENE

//OPERATION ORDER: SCALE -> ROTATE -> TRANSLATE
// TRANSLATE(ROTATE(SCALE(MATRIX)))

void MyApp::drawScene() {
    // Drawing directly in clip space
    Shaders->bind();


    // Pequeno Triângulo Vermelho 
    glm::mat4 model =
        glm::translate(glm::mat4(1.0f), glm::vec3(-0.311f, 0.111f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(-135.0f), glm::vec3(0, 0, 1)) * // (135º para a direita)
        glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 1.0f));

    glUniform4f(ColorId, 1.0f, 0.0f, 0.0f, 1.0f); 
    baseTriangle->draw(MatrixId, model);


    // Pequeno Triângulo Azul
    model = 
        glm::translate(glm::mat4(1.0f), glm::vec3(0.711f, 0.111f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(-135.0f), glm::vec3(0, 0, 1)) * 
        glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 1.0f));

    glUniform4f(ColorId, 0.0f, 0.502f, 0.502f, 1.0f); // azul
    baseTriangle->draw(MatrixId, model);


    // Médio Triângulo Roxo
    model =
        glm::translate(glm::mat4(1.0f), glm::vec3(0.8f, 0.2f, 0.0f)) *
        glm::rotate(glm::mat4(1.0f), glm::radians(135.0f), glm::vec3(0, 0, 1)) *
        glm::scale(glm::mat4(1.0f), glm::vec3(0.423f, 0.423f, 1.0f));

    glUniform4f(ColorId, 0.35f, 0.0f, 0.5f, 1.0f); // roxo
    baseTriangle->draw(MatrixId, model);


    // Quadrado Verde
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.7f, 0.2f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 1.0f));
    glUniform4f(ColorId, 0.1f, 0.8f, 0.2f, 1.0f); // verde
    baseSquare->draw(MatrixId, model);

    // Paralelogramo Laranja
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.4f, 0.2f, 0.0f))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f, 0.3f, 1.0f));
    glUniform4f(ColorId, 1.0f, 0.5f, 0.0f, 1.0f); // laranja
    baseParallelogram->draw(MatrixId, model);

    // Triângulo Rosa Largo
    model = glm::translate(glm::mat4(1.0f), glm::vec3(-0.1f, 0.5f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(-90.0f), glm::vec3(0, 0, 1))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 1.0f));
    glUniform4f(ColorId, 0.7f, 0.0f, 0.3f, 1.0f); // Rosa
    baseTriangle->draw(MatrixId, model);

    // Triângulo Azul-claro Largo
    model = glm::translate(glm::mat4(1.0f), glm::vec3(0.5f, -0.1f, 0.0f))
        * glm::rotate(glm::mat4(1.0f), glm::radians(90.0f), glm::vec3(0, 0, 1))
        * glm::scale(glm::mat4(1.0f), glm::vec3(0.6f, 0.6f, 1.0f));
    glUniform4f(ColorId, 0.3f, 0.4f, 1.0f, 1.0f); // Azul-Claro
    baseTriangle->draw(MatrixId, model);

    Shaders->unbind();
}

////////////////////////////////////////////////////////////////////// CALLBACKS


void MyApp::initCallback(GLFWwindow* win) {
    createShaderProgram(); 

    // Criar shapes base
    baseTriangle = new Shape(RightTriangleVertices, sizeof(RightTriangleVertices),
        RightTriangleIndices, sizeof(RightTriangleIndices));

    baseSquare = new Shape(SquareVertices, sizeof(SquareVertices),
        SquareIndices, sizeof(SquareIndices));

    baseParallelogram = new Shape(ParallelogramVertices, sizeof(ParallelogramVertices),
        ParallelogramIndices, sizeof(ParallelogramIndices));
}


void MyApp::windowCloseCallback(GLFWwindow* win) { destroyBufferObjects(); }

void MyApp::windowSizeCallback(GLFWwindow* win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
}

void MyApp::displayCallback(GLFWwindow* win, double elapsed) { drawScene(); }

/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char* argv[]) {
    mgl::Engine& engine = mgl::Engine::getInstance();
    engine.setApp(new MyApp());
    engine.setOpenGL(4, 6);
    engine.setWindow(600, 600, "Group 5 - Tortoise Tangram", 0, 1);
    engine.init();
    engine.run();
    exit(EXIT_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////// END
