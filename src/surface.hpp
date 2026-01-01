#pragma once
#include "vec3.hpp"
#include <vector>
#include <string>
#include <limits>

struct Pixel {
    float z = std::numeric_limits<float>::infinity();
    char c = ' ';
    int material = -1;
};

struct Triangle {
    Vec3 p1, p2, p3;
};

class Surface {
    int width, height;
    float logical_w, logical_h;
    float dx, dy;
    std::vector<Pixel> pixels;

public:
    Surface(int w, int h, float lw, float lh);
    
    void clear();
    void drawTriangle(const Triangle& tri, char c, int mat_idx);
    void print(bool color_support) const;
    void printNCurses(bool color_support) const;

private:
    int idxX(float x) const;
    int idxY(float y) const;
};