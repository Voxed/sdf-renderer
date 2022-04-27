//
// Created by voxed on 4/14/22.
//

#include <GL/glew.h>
#include "Primitive.h"

Primitive::Primitive(std::vector<glm::vec3> vertices, std::vector<unsigned int> indices, std::vector<glm::vec3> normals)
        : vertices(vertices),
          indices(indices),
          normals(normals) {
    // Calculate bounding box
    bounds[0] = glm::vec3(999999, 999999, 999999);
    bounds[1] = glm::vec3(-999999, -999999, -999999);
    for (const auto &vertex: vertices) {
        bounds[0] = glm::min(bounds[0], vertex);
        bounds[1] = glm::max(bounds[1], vertex);
    }

    for (const auto &i: indices) {
        orderedVertices.push_back(vertices[i]);
    }

    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    glGenBuffers(1, &vbo);
    glGenBuffers(1, &nbo);
    glGenBuffers(1, &ibo);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec3), vertices.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(0);

    glBindBuffer(GL_ARRAY_BUFFER, nbo);
    glBufferData(GL_ARRAY_BUFFER, normals.size() * sizeof(glm::vec3), normals.data(), GL_DYNAMIC_DRAW);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, nullptr);
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ibo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_DYNAMIC_DRAW);


    glBindVertexArray(0);
}

std::vector<glm::vec3> Primitive::Vertices() const {
    return orderedVertices;
}

std::array<glm::vec3, 2> Primitive::Bounds() const {
    return bounds;
}

void Primitive::Render() const {
    glBindVertexArray(vao);

    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, nullptr);

    glBindVertexArray(0);
}
