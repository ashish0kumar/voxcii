#include "surface.hpp"
#include <iostream>
#include <cmath>
#include <algorithm>
#include <array>
#include <ncurses.h>

Surface::Surface(int w, int h, float lw, float lh) 
    : width(w), height(h), logical_w(lw), logical_h(lh) {
    dx = logical_w / width;
    dy = logical_h / height;
    pixels.resize(width * height);
}

void Surface::clear() {
    std::fill(pixels.begin(), pixels.end(), Pixel{});
}

int Surface::idxX(float x) const {
    return std::clamp((int)std::floor(x / dx), 0, width - 1);
}

int Surface::idxY(float y) const {
    return std::clamp((int)std::floor(y / dy), 0, height - 1);
}

void Surface::drawTriangle(const Triangle& inTri, char c, int mat_idx) {
    // basic orientation culling
    if ((inTri.p2.x - inTri.p1.x) * (inTri.p3.y - inTri.p2.y) >= 
        (inTri.p3.x - inTri.p2.x) * (inTri.p2.y - inTri.p1.y)) return;

    // sort by X for scanning
    std::array<Vec3, 3> pts = {inTri.p1, inTri.p2, inTri.p3};
    std::sort(pts.begin(), pts.end(), [](const Vec3& a, const Vec3& b){ return a.x < b.x; });

    Vec3 normal = (inTri.p2 - inTri.p1).cross(inTri.p3 - inTri.p1).normalize();
    if (normal.z == 0) normal.z = 0.0001f; // prevent div by zero

    float xi = pts[0].x + dx/2.0f;
    float xf = pts[2].x - dx/2.0f;
    
    int x_start = idxX(xi);
    int x_end = idxX(xf);

    auto getY = [&](const Vec3& pA, const Vec3& pB, float x) {
        if (pA.x == pB.x) return pA.y;
        return pA.y + (pB.y - pA.y) * (x - pA.x) / (pB.x - pA.x);
    };

    for (int xx = x_start; xx <= x_end; ++xx) {
        float x = (xx + 0.5f) * dx;
        
        // scanline intersection
        float y1 = (x <= pts[1].x) ? getY(pts[0], pts[1], x) : getY(pts[1], pts[2], x);
        float y2 = getY(pts[0], pts[2], x);

        float yi = std::min(y1, y2);
        float yf = std::max(y1, y2);

        int y_start = idxY(yi + dy/2.0f);
        int y_end = idxY(yf - dy/2.0f);

        for (int yy = y_start; yy <= y_end; ++yy) {
            float y = (yy + 0.5f) * dy;
            float depth = pts[0].z - (normal.x * (x - pts[0].x) + normal.y * (y - pts[0].y)) / normal.z;

            Pixel& p = pixels[yy * width + xx];
            if (depth < p.z) {
                p.z = depth;
                p.c = c;
                p.material = mat_idx;
            }
        }
    }
}

void Surface::print(bool color_support) const {
    for (int y = 0; y < height; ++y) {
        for (int x = 0; x < width; ++x) {
            const auto& p = pixels[y * width + x];
            if (color_support && p.material != -1) {
                // ANSI color approximation
                std::cout << "\x1b[38;5;" << (p.material % 200 + 1) << "m" << p.c << "\x1b[0m";
            } else {
                std::cout << p.c;
            }
        }
        std::cout << "\n";
    }
}

void Surface::printNCurses(bool color_support) const {
    for (int y = 0; y < height; ++y) {
        move(y, 0);
        for (int x = 0; x < width; ++x) {
            const auto& p = pixels[y * width + x];
            if (color_support && p.material != -1) {
                int col = p.material + 1;
                attron(COLOR_PAIR(col));
                printw("%c", p.c);
                attroff(COLOR_PAIR(col));
            } else {
                printw("%c", p.c);
            }
        }
    }
}