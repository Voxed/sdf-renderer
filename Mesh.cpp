//
// Created by voxed on 4/14/22.
//

#include "Mesh.h"

Mesh::Mesh(std::vector<Primitive> primitives) : primitives(primitives) {
    for(const auto& p : primitives) {
        std::vector<glm::vec3> newVertices = p.Vertices();
        vertices.insert(vertices.end(), newVertices.begin(), newVertices.end());
    }

    bounds[0] = glm::vec3(999999, 999999, 999999);
    bounds[1] = glm::vec3(-999999, -999999, -999999);
    for (const auto &primitive: primitives) {
        bounds[0] = glm::min(primitive.Bounds()[0], bounds[0]);
        bounds[1] = glm::max(primitive.Bounds()[1], bounds[1]);
    }
}

std::array<glm::vec3, 2> Mesh::Bounds() const {
    std::array<glm::vec3, 2> b = bounds;

    // A slight margin.
    b[0] -= glm::vec3(0.2);
    b[1] += glm::vec3(0.2);

    return b;
}

std::vector<glm::vec3> Mesh::Vertices() const {
    return vertices;
}

glm::vec3 Mesh::Size() const {
    return Bounds()[1] - Bounds()[0];
}

std::vector<glm::vec3> Mesh::NormalizedVertices() const {
    std::vector<glm::vec3> normalized;
    for(const auto& vertex : vertices) {
        normalized.push_back((vertex - Bounds()[0])/Size());
    }
    return normalized;
}

void Mesh::Render() const {
    for(const auto& prim : primitives)
        prim.Render();
}
