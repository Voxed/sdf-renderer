//
// Created by voxed on 4/14/22.
//

#ifndef VX_LUMINENCE_MESH_H
#define VX_LUMINENCE_MESH_H


#include <vector>
#include "Primitive.h"
#include <GL/glew.h>

class Mesh {
    std::vector<Primitive> primitives;
    std::vector<glm::vec3> vertices;

    std::array<glm::vec3, 2> bounds;

public:
    Mesh(std::vector<Primitive> primitives);

    std::vector<glm::vec3> Vertices() const;

    std::vector<glm::vec3> NormalizedVertices() const;
    glm::vec3 Size() const;

    void Render() const;

    std::array<glm::vec3, 2> Bounds() const;
};


#endif //VX_LUMINENCE_MESH_H
