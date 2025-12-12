////////////////////////////////////////////////////////////////////////////////
//
//  Loading meshes from external files
//
// Copyright (c) 2023-25 by Carlos Martinho
//
// INTRODUCES:
// MODEL DATA, ASSIMP, mglMesh.hpp
//
////////////////////////////////////////////////////////////////////////////////
#define GLM_ENABLE_EXPERIMENTAL
#include "../mgl/mgl.hpp"
#include "OrbitalCamera.hpp"
#include "SceneNode.hpp"
#include <iostream>


////////////////////////////////////////////////////////////////////////// MYAPP

class MyApp : public mgl::App {
public:
  void initCallback(GLFWwindow *win) override;
  void displayCallback(GLFWwindow *win, double elapsed) override;
  void windowSizeCallback(GLFWwindow *win, int width, int height) override;
  void mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) override;
  void cursorCallback(GLFWwindow* win, double xpos, double ypos) override;
  void scrollCallback(GLFWwindow* win, double xoffset, double yoffset) override;
  void keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) override;

private:
  const GLuint UBO_BP = 0;
  mgl::ShaderProgram *Shaders = nullptr;
  mgl::Camera *Camera = nullptr;
  GLint ModelMatrixId;
  mgl::Mesh *Mesh = nullptr;
  SceneNode* rootNode = nullptr;

  void createMeshes();
  void createShaderPrograms();
  void createCamera();
  void drawScene();
};

OrbitalCamera* cam1;
OrbitalCamera* cam2;
OrbitalCamera* activeCam;

//For the callbacks
double lastX, lastY;
bool rightPressed = false;
bool leftPressed = false;

//Variaveis luz
glm::vec3 lightPos = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 0.6f, 0.3f);


///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    Mesh = new mgl::Mesh();
    Mesh->joinIdenticalVertices();
    Mesh->create("assets/models/coiledsword.obj");

    if (!Mesh->hasNormals())
        Mesh->generateNormals();
}


///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
    Shaders = new mgl::ShaderProgram();

    Shaders->addShader(GL_VERTEX_SHADER, "blinnPhong-vs.glsl");
    Shaders->addShader(GL_FRAGMENT_SHADER, "blinnPhong-fs.glsl");

    // Vertex attributes
    Shaders->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);

    if (Mesh->hasNormals()) {
        Shaders->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);
    }

    if (Mesh->hasTexcoords()) {
        Shaders->addAttribute(mgl::TEXCOORD_ATTRIBUTE, mgl::Mesh::TEXCOORD);
    }

    if (Mesh->hasTangentsAndBitangents()) {
        Shaders->addAttribute(mgl::TANGENT_ATTRIBUTE, mgl::Mesh::TANGENT);
    }

    // Uniforms
    Shaders->addUniform(mgl::MODEL_MATRIX);
    Shaders->addUniform("baseColor");

    // Lighting uniforms
    Shaders->addUniform("lightPos");
    Shaders->addUniform("lightColor");
    Shaders->addUniform("viewPos");

    // Material uniforms
    Shaders->addUniform("ambientStrength");
    Shaders->addUniform("specularStrength");
    Shaders->addUniform("shininess");

    // Camera UBO
    Shaders->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    Shaders->create();
    ModelMatrixId = Shaders->Uniforms[mgl::MODEL_MATRIX].index;
}


///////////////////////////////////////////////////////////////////////// CAMERA

// Eye(5,5,5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix1 =
    glm::lookAt(glm::vec3(5.0f, 5.0f, 5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Eye(-5,-5,-5) Center(0,0,0) Up(0,1,0)
const glm::mat4 ViewMatrix2 =
    glm::lookAt(glm::vec3(-5.0f, -5.0f, -5.0f), glm::vec3(0.0f, 0.0f, 0.0f),
                glm::vec3(0.0f, 1.0f, 0.0f));

// Orthographic LeftRight(-2,2) BottomTop(-2,2) NearFar(1,10)
const glm::mat4 ProjectionMatrix1 =
    glm::ortho(-2.0f, 2.0f, -2.0f, 2.0f, 1.0f, 10.0f);

// Perspective Fovy(30) Aspect(640/480) NearZ(1) FarZ(10)
const glm::mat4 ProjectionMatrix2 =
    glm::perspective(glm::radians(30.0f), 640.0f / 480.0f, 1.0f, 10.0f);

void MyApp::createCamera() {
  Camera = new mgl::Camera(UBO_BP);
//  Camera->setViewMatrix(ViewMatrix2);
//  Camera->setProjectionMatrix(ProjectionMatrix1);

  cam1 = new OrbitalCamera(glm::vec3(0, 0, 0), 10.0f);
  cam2 = new OrbitalCamera(glm::vec3(0, 0, 0), 15.0f);
  activeCam = cam1; 

  Camera->setViewMatrix(activeCam->getViewMatrix());
  Camera->setProjectionMatrix(activeCam->getProjectionMatrix(800.0f / 600.0f));
}

/////////////////////////////////////////////////////////////////////////// DRAW AND CONFIGURATIONS

glm::mat4 ModelMatrix(1.0f);

//enviar dados globais para o shader (luz e câmara)
void MyApp::drawScene() {
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    Shaders->bind();

    //  Luz da fogueira (posição no mundo)
    glUniform3fv(
        Shaders->Uniforms["lightPos"].index,
        1, glm::value_ptr(lightPos)
    );

    //  Cor da luz (fogo)
    glUniform3fv(
        Shaders->Uniforms["lightColor"].index,
        1, glm::value_ptr(lightColor)
    );

    //  Posição da câmara
    glm::vec3 camPos = activeCam->getPosition();
    glUniform3fv(
        Shaders->Uniforms["viewPos"].index,
        1, glm::value_ptr(camPos)
    );

    Shaders->unbind();
    rootNode->draw(glm::mat4(1.0f));
}




////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createShaderPrograms();
    createCamera();

    glm::vec3 bladeColor = glm::vec3(0.4f, 0.1f, 0.1f); 
    glm::vec3 handleColor = glm::vec3(0.6f, 0.1f, 0.2f);  

    rootNode = new SceneNode();
    
    // sword
    mgl::Mesh* swordMesh = Mesh;
    if (!swordMesh->hasNormals()) swordMesh->generateNormals();

    // Create one SceneNode per submesh
    for (size_t i = 0; i < swordMesh->getMeshCount(); i++) {
        SceneNode* partNode = new SceneNode();
        partNode->mesh = swordMesh;
        partNode->shader = Shaders;
        partNode->submeshIndex = i;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::scale(model, glm::vec3(0.05f)); // 5% do original
        //model = glm::translate(model, glm::vec3(0.0f, 0.0f, 0.0f));
        partNode->modelMatrix = model;


        // MATERIAIS DIFERENTES AQUI
        if (i == 0) { // blade
            partNode->color = bladeColor;
            partNode->ambientStrength = 0.08f;
            partNode->specularStrength = 0.8f;
            partNode->shininess = 4.0f;
        }
        else { // handle
            partNode->color = handleColor;
            partNode->ambientStrength = 0.15f;
            partNode->specularStrength = 0.1f;
            partNode->shininess = 8.0f;
        }

        rootNode->addChild(partNode);
    }


    // ==================== DEBUG LIGHT OBJECT ====================

    // Criar nó da luz
    SceneNode* lightNode = new SceneNode();

    // Criar mesh simples (esfera ou cubo)
    mgl::Mesh* lightMesh = new mgl::Mesh();
    lightMesh->create("assets/models/cube-v.obj");
    if (!lightMesh->hasNormals())
        lightMesh->generateNormals();

    lightNode->mesh = lightMesh;
    lightNode->shader = Shaders;

    // Transformação: posição + escala pequena
    glm::mat4 lightModel = glm::mat4(1.0f);
    lightModel = glm::translate(lightModel, lightPos);
    lightModel = glm::scale(lightModel, glm::vec3(0.05f));
    lightNode->modelMatrix = lightModel;

    // Material “emissivo” (não depende da luz)
    lightNode->color = glm::vec3(1.0f, 1.0f, 1.0f);
    lightNode->ambientStrength = 1.0f;
    lightNode->specularStrength = 0.0f;
    lightNode->shininess = 1.0f;

    // Adicionar à cena
    rootNode->addChild(lightNode);

}


void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
    float aspect = float(winx) / float(winy);
    Camera->setProjectionMatrix(activeCam->getProjectionMatrix(aspect));
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    drawScene(); 
}


void MyApp::mouseButtonCallback(GLFWwindow* win, int button, int action, int mods) {
    if (button == GLFW_MOUSE_BUTTON_RIGHT) {
        if (action == GLFW_PRESS) {
            rightPressed = true;
            glfwGetCursorPos(win, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE) {
            rightPressed = false;
        }
    }
    else if (button == GLFW_MOUSE_BUTTON_LEFT) {
        if (action == GLFW_PRESS) {
            leftPressed = true;
            glfwGetCursorPos(win, &lastX, &lastY);
        }
        else if (action == GLFW_RELEASE) {
            leftPressed = false;
        }
    }

}

void MyApp::cursorCallback(GLFWwindow* win, double xpos, double ypos) {
    float dx = float(xpos - lastX);
    float dy = float(ypos - lastY);

    if (rightPressed) {
        activeCam->rotate(-dx * 0.5f, -dy * 0.5f);
        Camera->setViewMatrix(activeCam->getViewMatrix());
    }

    lastX = xpos;
    lastY = ypos;
}


void MyApp::scrollCallback(GLFWwindow* win, double xoffset, double yoffset) {
    activeCam->zoom(-yoffset);
    Camera->setViewMatrix(activeCam->getViewMatrix());
}


void MyApp::keyCallback(GLFWwindow* win, int key, int scancode, int action, int mods) {
    int width, height;
    glfwGetWindowSize(win, &width, &height);


    //CHANGE BETWEEN CAMERAS
    if (glfwGetKey(win, GLFW_KEY_C) == GLFW_PRESS) {
        activeCam = (activeCam == cam1) ? cam2 : cam1;
        Camera->setViewMatrix(activeCam->getViewMatrix());
        Camera->setProjectionMatrix(activeCam->getProjectionMatrix((float)width / height));
    }

    //CHANGE PERPECTIVE/ORTHO 
    if (glfwGetKey(win, GLFW_KEY_P) == GLFW_PRESS) {
        activeCam->toggleProjection();
        Camera->setProjectionMatrix(activeCam->getProjectionMatrix((float)width / height));
    }
}


/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "CGJ Final Project - Ricardo Vieira", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
