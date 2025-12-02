#include "../mgl/mgl.hpp"
#include "OrbitalCamera.hpp"

//Functions definitions for OrbitalCamera (class declaration está no header)
OrbitalCamera::OrbitalCamera(const glm::vec3& c, float d)
    : center(c), distance(d), rotation(glm::quat(1, 0, 0, 0)) {}

glm::mat4 OrbitalCamera::getViewMatrix() const {
    glm::vec3 offset = rotation * glm::vec3(0, 0, distance);
    return glm::lookAt(center + offset, center, glm::vec3(0, 1, 0));
}

glm::mat4 OrbitalCamera::getProjectionMatrix(float aspect) const {
    if (perspective) {
        return glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    } else {
        float s = orthoSize;
        return glm::ortho(-s * aspect, s * aspect, -s, s, nearPlane, farPlane);
    }
}

void OrbitalCamera::rotate(float yawDeg, float pitchDeg) {
    glm::quat yaw = glm::angleAxis(glm::radians(yawDeg), glm::vec3(0, 1, 0));
    glm::quat pitch = glm::angleAxis(glm::radians(pitchDeg), glm::vec3(1, 0, 0));
    rotation = yaw * rotation * pitch;
}

void OrbitalCamera::zoom(float delta) {
    distance = glm::max(0.1f, distance + delta);
}

void OrbitalCamera::toggleProjection() {
    perspective = !perspective;
}

