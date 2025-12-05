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

    // Animation state (local matrices)
    glm::mat4 animStart = glm::mat4(1.0f);
    glm::mat4 animTarget = glm::mat4(1.0f);
    float animProgress = 0.0f; // 0..1
    int animDirection = 0; // -1 = towards start, +1 = towards target, 0 = stopped
    float animSpeed = 0.5f; // progress units per second

    SceneNode() = default;
    ~SceneNode() {
        for (auto child : children) delete child;
    }

    void addChild(SceneNode* child) { children.push_back(child); }

    // Set animation start/target in local space; optionally set progress and speed
    void setAnimationTargets(const glm::mat4& startLocal, const glm::mat4& targetLocal, float startProgress = 0.0f, float speed = 0.5f) {
        animStart = startLocal;
        animTarget = targetLocal;
        animProgress = glm::clamp(startProgress, 0.0f, 1.0f);
        animSpeed = speed;
        // keep modelMatrix in sync with progress
        modelMatrix = interpolateMatrix(animStart, animTarget, animProgress);
    }

    // command animation: -1 move to start, +1 move to target, 0 stop
    void commandAnimation(int direction) {
        animDirection = glm::clamp(direction, -1, 1);
    }

    // call every frame with elapsed seconds
    void updateAnimation(float deltaSeconds) {
        if (animDirection != 0) {
            animProgress += animDirection * animSpeed * deltaSeconds;
            animProgress = glm::clamp(animProgress, 0.0f, 1.0f);
            modelMatrix = interpolateMatrix(animStart, animTarget, animProgress);
            // if reached an end, stop (but leave progress at 0 or 1)
            if (animProgress <= 0.0f || animProgress >= 1.0f) animDirection = 0;
        }
        // propagate to children
        for (auto child : children) child->updateAnimation(deltaSeconds);
    }

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

private:
    // decompose two matrices and interpolate T, R (slerp) then recompose
    static glm::mat4 interpolateMatrix(const glm::mat4& A, const glm::mat4& B, float t) {
        // Decompose A
        glm::vec3 scaleA, translationA, skewA;
        glm::vec4 perspectiveA;
        glm::quat rotA;
        glm::decompose(A, scaleA, rotA, translationA, skewA, perspectiveA);

        // Decompose B
        glm::vec3 scaleB, translationB, skewB;
        glm::vec4 perspectiveB;
        glm::quat rotB;
        glm::decompose(B, scaleB, rotB, translationB, skewB, perspectiveB);

        // Interpolate translation & rotation only
        glm::vec3 trans = glm::mix(translationA, translationB, t);
        glm::quat rot = glm::slerp(rotA, rotB, t);

        glm::mat4 T = glm::translate(glm::mat4(1.0f), trans);
        glm::mat4 R = glm::toMat4(rot);

        // NO SCALE
        return T * R;
    }
};
