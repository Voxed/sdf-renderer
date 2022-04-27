//
// Created by voxed on 4/14/22.
//

#ifndef VX_LUMINENCE_PRIMITIVE_H
#define VX_LUMINENCE_PRIMITIVE_H


#include <vector>
#include <glm/vec3.hpp>
#include <glm/glm.hpp>
#include <array>
#include <GL/glew.h>

class Primitive {
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> orderedVertices;
    std::vector<unsigned int> indices;
    std::vector<glm::vec3> normals;

    GLuint vao;
    GLuint vbo;
    GLuint ibo;
    GLuint nbo;

    std::array<glm::vec3, 2> bounds;

public:
    Primitive(std::vector<glm::vec3> vertices, std::vector<unsigned int> indices, std::vector<glm::vec3> normals);

    std::vector<glm::vec3> Vertices() const;

    std::array<glm::vec3, 2> Bounds() const;

    void Render() const;
};


#endif //VX_LUMINENCE_PRIMITIVE_H
