// OrbitalCamera.hpp
#pragma once
#include "../mgl/mgl.hpp"

class OrbitalCamera {
public:
    glm::vec3 center;
    float distance;

    glm::quat rotation;
    bool perspective = true;

    float fov = 45.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float orthoSize = 10.0f;

    OrbitalCamera(const glm::vec3& c, float d);

    glm::mat4 getViewMatrix() const;
    glm::mat4 getProjectionMatrix(float aspect) const;
    glm::vec3 getPosition() const;

    void rotate(float yawDeg, float pitchDeg);
    void zoom(float delta);
    void toggleProjection();
};
