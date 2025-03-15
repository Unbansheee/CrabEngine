module;
#pragma once

//#include <glm/vec2.hpp>
//#include <glm/vec3.hpp>

export module mesh_vertex;
import glm;

export struct MeshVertex {
    MeshVertex() = default;
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec3 color;
    glm::vec2 uv;
    glm::vec3 tangent;
    glm::vec3 bitangent;
};

