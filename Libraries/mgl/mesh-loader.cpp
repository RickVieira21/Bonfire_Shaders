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
#define STB_IMAGE_IMPLEMENTATION
#include "../stb/stb_image.h"

#include "../mgl/mgl.hpp"
#include "OrbitalCamera.hpp"
#include "SceneNode.hpp"
#include "Particle.hpp"
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
glm::vec3 lightPos = glm::vec3(10.0f, 0.0f, 0.0f);
glm::vec3 lightColor = glm::vec3(1.0f, 1.0f, 1.0f);

//Skybox
mgl::Mesh* skyboxMesh = nullptr;
mgl::ShaderProgram* skyboxShader = nullptr;
GLuint skyboxCubemap = 0;

//Procedural
mgl::ShaderProgram* ashShader = nullptr;
mgl::ShaderProgram* stonesShader = nullptr;

//Particles
mgl::ShaderProgram* fireShader = nullptr;
static const int MAX_PARTICLES = 500;

std::vector<Particle> particles;
GLuint particleVAO, particleVBO;



///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
    Mesh = new mgl::Mesh();
    Mesh->joinIdenticalVertices();
    Mesh->create("assets/models/coiledsword.obj");

    if (!Mesh->hasNormals())
        Mesh->generateNormals();


    // Mesh da skybox 
    skyboxMesh = new mgl::Mesh();
    skyboxMesh->create("assets/models/cube-v.obj");
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



    // ==================== SKYBOX SHADER ====================

    skyboxShader = new mgl::ShaderProgram();
    skyboxShader->addShader(GL_VERTEX_SHADER, "skybox-vs.glsl");
    skyboxShader->addShader(GL_FRAGMENT_SHADER, "skybox-fs.glsl");

    skyboxShader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);

    skyboxShader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    skyboxShader->addUniform("skybox");

    skyboxShader->create();


    // ==================== ASH PROCEDURAL SHADER ====================

    ashShader = new mgl::ShaderProgram();
    ashShader->addShader(GL_VERTEX_SHADER, "procedural-vs.glsl");
    ashShader->addShader(GL_FRAGMENT_SHADER, "ash-fs.glsl");

    ashShader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    ashShader->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);

    ashShader->addUniform(mgl::MODEL_MATRIX);
    ashShader->addUniform("lightPos");
    ashShader->addUniform("lightColor");
    ashShader->addUniform("viewPos");

    ashShader->addUniform("ambientStrength");
    ashShader->addUniform("specularStrength");
    ashShader->addUniform("shininess");

    ashShader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    ashShader->create();


    // ==================== STONES PROCEDURAL SHADER ====================

    stonesShader = new mgl::ShaderProgram();
    stonesShader->addShader(GL_VERTEX_SHADER, "procedural-vs.glsl");
    stonesShader->addShader(GL_FRAGMENT_SHADER, "stones-fs.glsl");

    stonesShader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);
    stonesShader->addAttribute(mgl::NORMAL_ATTRIBUTE, mgl::Mesh::NORMAL);

    stonesShader->addUniform(mgl::MODEL_MATRIX);
    stonesShader->addUniform("lightPos");
    stonesShader->addUniform("lightColor");
    stonesShader->addUniform("viewPos");

    stonesShader->addUniform("ambientStrength");
    stonesShader->addUniform("specularStrength");
    stonesShader->addUniform("shininess");

    stonesShader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);
    stonesShader->create();


    // ==================== FIRE PARTICLE SHADER ====================

    fireShader = new mgl::ShaderProgram();

    // Shaders
    fireShader->addShader(GL_VERTEX_SHADER, "fire-vs.glsl");
    fireShader->addShader(GL_GEOMETRY_SHADER, "fire-gs.glsl");
    fireShader->addShader(GL_FRAGMENT_SHADER, "fire-fs.glsl");

    // Attributes (particles = pontos)
    fireShader->addAttribute(mgl::POSITION_ATTRIBUTE, mgl::Mesh::POSITION);

    // Uniforms
    fireShader->addUniform("time");
    fireShader->addUniform("fireColor");     // opcional
    fireShader->addUniform("particleSize");  // opcional

    // Camera UBO (OBRIGATÓRIO)
    fireShader->addUniformBlock(mgl::CAMERA_BLOCK, UBO_BP);

    // Criar programa
    fireShader->create();

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
    
    
    // ==================== SKYBOX ====================
    glDepthFunc(GL_LEQUAL);
    glDisable(GL_CULL_FACE);

    skyboxShader->bind();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, skyboxCubemap);
    glUniform1i(skyboxShader->Uniforms["skybox"].index, 0);

    skyboxMesh->draw();

    skyboxShader->unbind();

    glDepthFunc(GL_LESS);
    glEnable(GL_CULL_FACE);
  
    Shaders->unbind();
    

    // ==================== ASH PROCEDURAL ====================

    ashShader->bind();
    glUniform3fv(ashShader->Uniforms["lightPos"].index, 1, glm::value_ptr(lightPos));
    glUniform3fv(ashShader->Uniforms["lightColor"].index, 1, glm::value_ptr(lightColor));
    glUniform3fv(ashShader->Uniforms["viewPos"].index, 1, glm::value_ptr(camPos));
    ashShader->unbind();


    // ==================== STONES PROCEDURAL ====================

    stonesShader->bind();
    glUniform3fv(stonesShader->Uniforms["lightPos"].index, 1, glm::value_ptr(lightPos));
    glUniform3fv(stonesShader->Uniforms["lightColor"].index, 1, glm::value_ptr(lightColor));
    glUniform3fv(stonesShader->Uniforms["viewPos"].index, 1, glm::value_ptr(camPos));
    stonesShader->unbind();

    rootNode->draw(glm::mat4(1.0f));


    // ==================== FIRE ====================

    if (!particles.empty() && particleVAO != 0) {

        fireShader->bind();

        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE);
        glDepthMask(GL_FALSE);

        glBindVertexArray(particleVAO);
        glDrawArrays(GL_POINTS, 0, particles.size());
        glBindVertexArray(0);

        glDepthMask(GL_TRUE);
        glDisable(GL_BLEND);

        fireShader->unbind();
    }


}



////////////////////////////////////////////////////////////////////// SKYBOX METHODS

GLuint loadCubemapFromCross(const std::string& filename) {
    int width, height, channels;
    unsigned char* data = stbi_load(filename.c_str(), &width, &height, &channels, 0);

    if (!data) {
        std::cout << "Failed to load skybox cross image\n";
        return 0;
    }

    int faceSize = width / 4;
    GLenum format = (channels == 4) ? GL_RGBA : GL_RGB;

    GLuint texID;
    glGenTextures(1, &texID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, texID);

    // offsets (x, y) das faces na imagem
    struct Face { int x, y; };
    Face faces[6] = {
        {2, 1}, // +X (right)
        {0, 1}, // -X (left)
        {1, 0}, // +Y (top)
        {1, 2}, // -Y (bottom)
        {1, 1}, // +Z (front)
        {3, 1}  // -Z (back)
    };

    for (int i = 0; i < 6; i++) {
        unsigned char* faceData = new unsigned char[faceSize * faceSize * channels];

        for (int y = 0; y < faceSize; y++) {
            for (int x = 0; x < faceSize; x++) {
                int srcX = faces[i].x * faceSize + x;
                int srcY = faces[i].y * faceSize + y;

                for (int c = 0; c < channels; c++) {
                    faceData[(y * faceSize + x) * channels + c] =
                        data[(srcY * width + srcX) * channels + c];
                }
            }
        }

        glTexImage2D(
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
            0, format,
            faceSize, faceSize,
            0, format, GL_UNSIGNED_BYTE,
            faceData
        );

        delete[] faceData;
    }

    stbi_image_free(data);

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return texID;
}



// ==================== FIRE METHODS ====================

void initParticles() {

    particles.resize(MAX_PARTICLES);

    for (auto& p : particles) {
        p.position = glm::vec3(0.0f);
        p.velocity = glm::vec3(
            (rand() / float(RAND_MAX) - 0.5f) * 0.3f,
            1.0f + rand() / float(RAND_MAX),
            (rand() / float(RAND_MAX) - 0.5f) * 0.3f
        );
        p.life = rand() / float(RAND_MAX);
    }

    glGenVertexArrays(1, &particleVAO);
    glGenBuffers(1, &particleVBO);

    glBindVertexArray(particleVAO);
    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);

    glBufferData(
        GL_ARRAY_BUFFER,
        particles.size() * sizeof(Particle),
        particles.data(),
        GL_DYNAMIC_DRAW
    );

    // position
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0, 3, GL_FLOAT, GL_FALSE,
        sizeof(Particle),
        (void*)0
    );

    // life
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1, 1, GL_FLOAT, GL_FALSE,
        sizeof(Particle),
        (void*)(offsetof(Particle, life))
    );

    glBindVertexArray(0);
}



void updateParticles(double elapsed) {

    float dt = float(elapsed);

    for (auto& p : particles) {
        p.life += dt * 0.5f;

        if (p.life > 1.0f) {
            p.life = 0.0f;
            p.position = glm::vec3(0.0f);
        }

        p.position += p.velocity * dt;
    }

    glBindBuffer(GL_ARRAY_BUFFER, particleVBO);
    glBufferSubData(
        GL_ARRAY_BUFFER,
        0,
        particles.size() * sizeof(Particle),
        particles.data()
    );
}



////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createShaderPrograms();
    createCamera();
    initParticles(); // função que cria VAO/VBO

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

    // Criar cubo simples 
    mgl::Mesh* lightMesh = new mgl::Mesh();
    lightMesh->create("assets/models/cube-v.obj");
    if (!lightMesh->hasNormals())
        lightMesh->generateNormals();

    lightNode->mesh = lightMesh;
    lightNode->shader = Shaders;

    // Transformações...
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


    // ==================== SKYBOX ====================

    skyboxCubemap = loadCubemapFromCross(
        "assets/skybox/skybox.png"
    );

    // ==================== Ash Procedural ====================

    SceneNode* ashNode = new SceneNode();
    ashNode->mesh = new mgl::Mesh();
    ashNode->mesh->create("assets/models/ash.obj");

    ashNode->shader = ashShader;

    //Transformações...
    glm::mat4 ashModel = glm::mat4(1.0f);
    ashModel = glm::scale(ashModel, glm::vec3(1.0f));
    ashNode->modelMatrix = ashModel;

    ashNode->ambientStrength = 0.7f;
    ashNode->specularStrength = 0.5f;
    ashNode->shininess = 1.0f;

    rootNode->addChild(ashNode);


    // ==================== Stone Procedural ====================

    mgl::Mesh* stoneMesh = new mgl::Mesh();
    stoneMesh->create("assets/models/stone.obj");

    if (!stoneMesh->hasNormals())
        stoneMesh->generateNormals();

    int stoneCount = 12;
    float radius = 1.2f;

    for (int i = 0; i < stoneCount; i++) {

        SceneNode* stoneNode = new SceneNode();
        stoneNode->mesh = stoneMesh;
        stoneNode->shader = stonesShader;

        float angle = (2.0f * 3.1415f * i) / stoneCount;

        float x = cos(angle) * radius;
        float z = sin(angle) * radius;

        // Pequena variação aleatória (de scale e rotation)
        float scale = 0.18f + 0.05f * (rand() / float(RAND_MAX));
        float rotation = rand() / float(RAND_MAX) * 360.0f;

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::translate(model, glm::vec3(x, -0.4f, z));
        model = glm::rotate(model, glm::radians(rotation), glm::vec3(0, 1, 0));
        model = glm::scale(model, glm::vec3(scale));

        stoneNode->modelMatrix = model;

        // Material
        stoneNode->ambientStrength = 0.12f;
        stoneNode->specularStrength = 0.25f;
        stoneNode->shininess = 16.0f;

        rootNode->addChild(stoneNode);
    }

}



void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
    float aspect = float(winx) / float(winy);
    Camera->setProjectionMatrix(activeCam->getProjectionMatrix(aspect));
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    updateParticles(elapsed); //Atualização por frame
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
