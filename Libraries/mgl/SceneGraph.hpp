#pragma once
#include "../mgl/mgl.hpp"
#include <vector>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <glm/gtx/quaternion.hpp>

class SceneNode {
public:
    mgl::Mesh* mesh = nullptr;
    mgl::ShaderProgram* shader = nullptr;

    glm::mat4 modelMatrix = glm::mat4(1.0f); // local transform
    glm::vec3 color = glm::vec3(1.0f);       // base color

    // Material (Blinn-Phong)
    float ambientStrength = 0.1f;
    float specularStrength = 0.5f;
    float shininess = 32.0f;

    std::vector<SceneNode*> children;
    int submeshIndex = -1; // -1 = draw all submeshes

    SceneNode() = default;

    ~SceneNode() {
        for (auto child : children) delete child;
    }

    void addChild(SceneNode* child) {
        children.push_back(child);
    }

    //antes de desenhar, enviamos os dados ao shader - “automatically handles matrices”
    void draw(const glm::mat4& parentMatrix) {
        glm::mat4 globalMatrix = parentMatrix * modelMatrix;

        //“handles shaders”
        if (mesh && shader) {
            shader->bind();

            // Model matrix
            GLint modelId = shader->Uniforms[mgl::MODEL_MATRIX].index;
            if (modelId >= 0) {
                glUniformMatrix4fv(
                    modelId, 1, GL_FALSE,
                    glm::value_ptr(globalMatrix)
                );
            }

            // Base color
            GLint colorId = shader->Uniforms["baseColor"].index;
            if (colorId >= 0) {
                glUniform3fv(colorId, 1, glm::value_ptr(color));
            }

            // Material uniforms
            GLint ambId = shader->Uniforms["ambientStrength"].index;
            GLint specId = shader->Uniforms["specularStrength"].index;
            GLint shinId = shader->Uniforms["shininess"].index;

            if (ambId >= 0) glUniform1f(ambId, ambientStrength);
            if (specId >= 0) glUniform1f(specId, specularStrength);
            if (shinId >= 0) glUniform1f(shinId, shininess);

            // Draw mesh
            if (submeshIndex >= 0) {
                mesh->draw(submeshIndex);
            }
            else {
                mesh->draw();
            }

            shader->unbind();
        }

        // Draw children
        for (auto child : children) {
            child->draw(globalMatrix);
        }
    }
};
