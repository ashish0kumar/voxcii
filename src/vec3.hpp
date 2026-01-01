#pragma once
#include <cmath>
#include <algorithm>

struct Vec3 {
    float x{0}, y{0}, z{0};

    Vec3() = default;
    Vec3(float x, float y, float z) : x(x), y(y), z(z) {}

    float mag() const { 
        return std::sqrt(x*x + y*y + z*z); 
    }

    Vec3 normalize() const {
        float m = mag();
        return (m == 0) ? Vec3{} : Vec3{x/m, y/m, z/m};
    }

    float dot(const Vec3& other) const {
        return x * other.x + y * other.y + z * other.z;
    }

    Vec3 cross(const Vec3& other) const {
        return {
            y * other.z - z * other.y, 
            z * other.x - x * other.z, 
            x * other.y - y * other.x
        };
    }

    Vec3 operator+(const Vec3& o) const { 
        return {
            x + o.x, 
            y + o.y, 
            z + o.z
        };
    }
    Vec3 operator-(const Vec3& o) const { 
        return {
            x - o.x, 
            y - o.y, 
            z - o.z
        }; 
    }
    Vec3 operator*(float s) const { 
        return {
            x * s, 
            y * s, 
            z * s
        }; 
    }
    
    Vec3 rotateY(float cos_a, float sin_a) const {
        return {
            x * cos_a - z * sin_a, 
            y, 
            x * sin_a + z * cos_a
        };
    }

    Vec3 rotateX(float cos_a, float sin_a) const {
        return {
            x, 
            y * cos_a - z * sin_a, 
            y * sin_a + z * cos_a
        };
    }
};