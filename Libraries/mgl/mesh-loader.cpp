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
  SceneNode* tableNode = nullptr;

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
std::vector<SceneNode*> pieceNodes; // filled on initCallback


///////////////////////////////////////////////////////////////////////// MESHES

void MyApp::createMeshes() {
  std::string mesh_dir = "assets/models/";
  std::string mesh_file = "Pickagram_Group05.obj";
  std::string mesh_fullname = mesh_dir + mesh_file;

  Mesh = new mgl::Mesh();
  Mesh->joinIdenticalVertices();
  Mesh->create(mesh_fullname);
}

///////////////////////////////////////////////////////////////////////// SHADER

void MyApp::createShaderPrograms() {
  Shaders = new mgl::ShaderProgram();
  Shaders->addShader(GL_VERTEX_SHADER, "cube-vs.glsl");
  Shaders->addShader(GL_FRAGMENT_SHADER, "cube-fs.glsl");

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

  Shaders->addUniform(mgl::MODEL_MATRIX);
  Shaders->addUniform("baseColor");               // per-node base color used by fragment shader
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

void MyApp::drawScene() {
  //Shaders->bind();
  //glUniformMatrix4fv(ModelMatrixId, 1, GL_FALSE, glm::value_ptr(ModelMatrix));
  //Mesh->draw();
  //Shaders->unbind();
  rootNode->draw(glm::mat4(1.0f));
}


void createConfigurations() {
    std::vector<glm::mat4> pickagramMatrices;
    for (auto p : pieceNodes) {
        pickagramMatrices.push_back(p->modelMatrix); 
    }

    struct RotationInstruction {
        glm::vec3 axis;
        float degrees;
        int repeatCount;
    };

    // box configuration: example layout on XZ plane (positions are local to table)
    std::vector<glm::mat4> boxMatrices;
    // Final positions of all pickagram shapes
    std::vector<glm::vec3> boxPositions = {
        //Small Triangle 1
        {-0.7f, 1.2f, 0.06f},
        //Large Triangle 1
        {0.78f, 0.6f, 0.0f}, 
        //Parallelogram
        {-0.5f, 0.6f, -0.6f},
        //Small Triangle 2
        {1.1f, 0.9f, 0.0f},
        //Medium Triangle
        {0.38f, 1.03f, 0.0f}, 
        //Square
        {-0.5f, 1.0f, 0.0f},
        //Large Triangle 2
        {-0.1f, 1.45f, -0.69f}
    };
    std::vector<std::vector<RotationInstruction>> pieceRotationSequences = {

        //Small Triangle 1
        {
            { {1,0,0}, 90.0f, 1 }
        },
        //Large Triangle 1
        {
            { {1,0,0}, 90.0f, 1 },
            { {0,0,1}, 45.0f, 1 },
        },
        //Parallelogram
        {
            { {1,0,0}, 90.0f, 1 },
            { {0,1,0}, 90.0f, 2 },
            { {0,0,1}, 45.0f, 1 },
        },
        //Small Triangle 2
        {
            { {0,0,1}, 90.0f, 1 },
            { {0,1,0}, 90.0f, 1 }
        },
        //Medium Triangle
        {
            { {0,1,0}, 135.0f, 1 },
            { {1,0,0}, 90.0f, 1 },
        },
        //Square
        {
            { {0,1,0}, -135.0f, 1 },
            { {1,0,0}, -90.0f, 1 },
            { {0,1,0}, 180.0f, 1 },
            { {1,0,0}, 180.0f, 1 },
            { {0,1,0}, 180.0f, 1 },
        },
        //Large Triangle 2
        {
            { {1,0,0}, 90.0f, 1 },
            { {0,0,1}, -45.0f, 1 },
            { {0,1,0}, 180.0f, 1 },
            { {0,0,1}, 90.0f, 1 },
        }
    };



    for (size_t i = 0; i < pieceNodes.size(); ++i) {
        glm::vec3 pos = boxPositions[i % boxPositions.size()];
        glm::mat4 M = glm::translate(glm::mat4(1.0f), pos);
        glm::mat4 R = glm::mat4(1.0f);
        //
        // Get this piece’s rotation steps
        const auto& steps = pieceRotationSequences[i];

        // Apply all steps in order
        for (const RotationInstruction& step : steps) {
            glm::vec3 axis = glm::normalize(step.axis);

            for (int k = 0; k < step.repeatCount; ++k) {
                R = glm::rotate(glm::mat4(1.0f), glm::radians(step.degrees), axis) * R;
            }
        }
        //glm::mat4 R = glm::rotate(glm::mat4(1.0f), glm::radians(boxRadiansRotations[i]), boxAxisRotations[i]);
        glm::mat4 finalM = M * R;
        boxMatrices.push_back(finalM);
    }

    // Assign animation start/target for each piece:
    for (size_t i = 0; i < pieceNodes.size(); ++i) {
        SceneNode* node = pieceNodes[i];
        glm::mat4 pickM = pickagramMatrices[i];
        glm::mat4 boxM = boxMatrices[i];

        node->setAnimationTargets(pickM, boxM, /*startProgress=*/1.0f, /*speed=*/0.8f);
        //startProgress = 1 means currently at pickagram config.
    }
}



////////////////////////////////////////////////////////////////////// CALLBACKS

void MyApp::initCallback(GLFWwindow* win) {
    createMeshes();
    createShaderPrograms();
    createCamera();

    std::vector<glm::vec3> pickagramColors = {
    //Small Triangle 1
    {0.0f, 0.502f, 1.0f},
    //Large Triangle 1
    {0.3f, 0.4f, 1.0f},
    //Parallelogram
    {1.0f, 0.5f, 0.0f},
    //Small Triangle 2
    {1.0f, 0.0f, 0.0f},
    //Medium Triangle
    {0.35f, 0.0f, 0.5f},
    //Square
    {0.1f, 0.8f, 0.2f},
    //Large Triangle 2
    {0.7f, 0.0f, 0.3f},
    };

    // Table
    mgl::Mesh* tableMesh = new mgl::Mesh();
    tableMesh->create("assets/models/tabletop.obj");

    tableNode = new SceneNode();
    tableNode->mesh = tableMesh;
    tableNode->shader = Shaders;
    tableNode->color = glm::vec3(0.6f);

    rootNode = new SceneNode();
    rootNode->addChild(tableNode);

    // Pickagram
    mgl::Mesh* pickagramMesh = new mgl::Mesh();
    pickagramMesh->create("assets/models/Pickagram_Group05.obj");
    if (!pickagramMesh->hasNormals()) pickagramMesh->generateNormals();

    // Create one SceneNode per submesh
    for (size_t i = 0; i < pickagramMesh->getMeshCount(); i++) {
        SceneNode* partNode = new SceneNode();
        partNode->mesh = pickagramMesh;
        partNode->shader = Shaders;
        partNode->submeshIndex = i;
        partNode->modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(0.0f, 0.7f, 0.0f));
        partNode->color = pickagramColors[i];
        tableNode->addChild(partNode);
        pieceNodes.push_back(partNode);
    }

    createConfigurations();
}


void MyApp::windowSizeCallback(GLFWwindow *win, int winx, int winy) {
    glViewport(0, 0, winx, winy);
    float aspect = float(winx) / float(winy);
    Camera->setProjectionMatrix(activeCam->getProjectionMatrix(aspect));
}

void MyApp::displayCallback(GLFWwindow *win, double elapsed) { 
    rootNode->updateAnimation((float)elapsed);
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

    if (leftPressed && tableNode) {
        // Camera axes (use the active camera view matrix)
        glm::vec3 right = glm::normalize(glm::vec3(activeCam->getViewMatrix()[0]));
        glm::vec3 forward = -glm::normalize(glm::vec3(activeCam->getViewMatrix()[2]));

        const float scale = 0.01f;
        tableNode->modelMatrix = glm::translate(
            tableNode->modelMatrix,
            right * dx * scale + forward * (-dy) * scale
        );
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

    // left arrow: animate toward box (progress -> 0), right arrow: animate toward pickagram (progress -> 1)
    if (action == GLFW_PRESS || action == GLFW_REPEAT) {
        if (key == GLFW_KEY_LEFT) {
            std::cout << "[LOG] LEFT key pressed -> animating toward box\n";
            for (auto n : pieceNodes) n->commandAnimation(-1);
        }
        if (key == GLFW_KEY_RIGHT) {
            std::cout << "[LOG] RIGHT key pressed -> animating toward pickagram\n";
            for (auto n : pieceNodes) n->commandAnimation(+1);
        }
    }
    else if (action == GLFW_RELEASE) {
        if (key == GLFW_KEY_LEFT || key == GLFW_KEY_RIGHT) {
            std::cout << "[LOG] Key released -> stopping animation\n";
            for (auto n : pieceNodes) n->commandAnimation(0);
        }
    }
}


/////////////////////////////////////////////////////////////////////////// MAIN

int main(int argc, char *argv[]) {
  mgl::Engine &engine = mgl::Engine::getInstance();
  engine.setApp(new MyApp());
  engine.setOpenGL(4, 6);
  engine.setWindow(800, 600, "3D Pickagram - Group 5", 0, 1);
  engine.init();
  engine.run();
  exit(EXIT_SUCCESS);
}

////////////////////////////////////////////////////////////////////////////////
