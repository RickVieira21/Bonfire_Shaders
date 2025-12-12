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
    glm::mat4 modelMatrix = glm::mat4(1.0f); // local transform (relative to parent)
    glm::vec3 color = glm::vec3(0.0f); // base color
    std::vector<SceneNode*> children;
    int submeshIndex = -1; // draw all meshes if -1, or a specific submesh

    SceneNode() = default;
    ~SceneNode() {
        for (auto child : children) delete child;
    }

    void addChild(SceneNode* child) { children.push_back(child); }

    // Draw as before
    void draw(const glm::mat4& parentMatrix) {
        glm::mat4 globalMatrix = parentMatrix * modelMatrix;

        if (mesh && shader) {
            shader->bind();

            // ModelMatrix uniform
            GLint modelId = shader->Uniforms[mgl::MODEL_MATRIX].index;
            if (modelId >= 0) glUniformMatrix4fv(modelId, 1, GL_FALSE, glm::value_ptr(globalMatrix));

            // Set base color uniform
            GLint colorId = shader->Uniforms["baseColor"].index;
            if (colorId >= 0)
                glUniform3fv(colorId, 1, glm::value_ptr(color));

            // Draw single mesh or all
            if (submeshIndex >= 0) mesh->draw(submeshIndex);
            else mesh->draw();

            shader->unbind();
        }

        for (auto child : children) {
            child->draw(globalMatrix);
        }
    }
};
