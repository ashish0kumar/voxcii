#pragma once
#include "vec3.hpp"
#include <vector>
#include <string>
#include <array>

struct Face {
    std::array<int, 3> idxs;
    int material_idx{-1};
};

struct Material {
    std::string name;
    float kd[3]{1.0f, 1.0f, 1.0f};
};

class Model {
public:
    std::vector<Vec3> vertices;
    std::vector<Face> faces;
    std::vector<Material> materials;

    static Model loadFromObj(const std::string& filename, bool use_colors);
    static Model loadFromStl(const std::string& filename);

    void normalize();
    void invertTriangles();
    void transform(int axis1, int axis2, int axis3, bool invX, bool invY, bool invZ);
    int getMaterialIdx(const std::string& name) const;
};