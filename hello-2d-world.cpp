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

    void createShaderProgram();
    void createBufferObjects();
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

    Shaders->create();

    MatrixId = Shaders->Uniforms["Matrix"].index;
}

//////////////////////////////////////////////////////////////////// VAOs & VBOs

typedef struct {
    GLfloat XYZW[4];
    GLfloat RGBA[4];
} Vertex;

const Vertex Vertices[] = {
    {{0.25f, 0.25f, 0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
    {{0.75f, 0.25f, 0.0f, 1.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
    {{0.50f, 0.75f, 0.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}} };

const GLubyte Indices[] = { 0, 1, 2 };

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

//Identity Matrix
//const glm::mat4 I(1.0f);

//Orange Triangle (DONE)
const glm::mat4 OrangeTri =
glm::translate(glm::scale(glm::mat4(1.0f), glm::vec3(1.2f, 0.8f, 1.0f)),
    glm::vec3(-0.7f, -1.0f, 1.0f));

//Large Blue Triangle (TODO)
const glm::mat4 LargeBlueTri =
glm::translate(glm::mat4(1.0f), glm::vec3(-1.0f, 0.0f, 0.0f));

//Large Pink Triangle (TODO)
const glm::mat4 LargePinkTri =
glm::translate(
    glm::rotate(glm::mat4(1.0f), 45.0f, glm::vec3(1.0f, 1.0f, 1.0f)),
    glm::vec3(0.0f,-1.0f,0.0f));
//glm::rotate(glm::scale(glm::mat4(1.0f), glm::vec3(1.0f, 1.0f, 1.0f)),
//    glm::radians(60.0f),
//    glm::vec3(1.0f,1.0f,0.0f));


void MyApp::drawScene() {
    // Drawing directly in clip space

    glBindVertexArray(VaoId);
    Shaders->bind();

    //Test included in base file
    //acts as example of how to display a triangle on the clip space
    //glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(I));
    //glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE,
    //    reinterpret_cast<GLvoid*>(0));

    //Drawing Orange Triangle
    glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(OrangeTri));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE,
        reinterpret_cast<GLvoid*>(0));

    //Drawing Large Blue Triangle
    glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(LargeBlueTri));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE,
        reinterpret_cast<GLvoid*>(0));

    //Drawing Large Pink Triangle
    glUniformMatrix4fv(MatrixId, 1, GL_FALSE, glm::value_ptr(LargePinkTri));
    glDrawElements(GL_TRIANGLES, 3, GL_UNSIGNED_BYTE,
        reinterpret_cast<GLvoid*>(0));

    Shaders->unbind();
    glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createBufferObjects();
    createShaderProgram();
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
    engine.setWindow(600, 600, "Hello Modern 2D World", 0, 1);
    engine.init();
    engine.run();
    exit(EXIT_SUCCESS);
}

//////////////////////////////////////////////////////////////////////////// END
