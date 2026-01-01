#include "model.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <limits>
#include <cstring>
#include <cmath>

namespace {
    float triArea(const Vec3& p1, const Vec3& p2, const Vec3& p3) {
        return std::abs(p1.x*(p2.y - p3.y) + p2.x*(p3.y - p1.y) + p3.x*(p1.y - p2.y)) / 2.0f;
    }

    bool pointInTriangle(const Vec3& pt, const Vec3& v1, const Vec3& v2, const Vec3& v3) {
        float total = triArea(v1, v2, v3);
        float a1 = triArea(v1, v2, pt);
        float a2 = triArea(v2, v3, pt);
        float a3 = triArea(v3, v1, pt);
        return (a1 + a2 + a3) <= total * 1.00001f;
    }

    void triangularizeRecurse(std::vector<Vec3>& vecs, std::vector<int>& idxs, bool orient, std::vector<int>& out) {
        size_t n = vecs.size();
        if (n < 3) return;
        if (n == 3) {
            out.insert(out.end(), {idxs[0], idxs[1], idxs[2]});
            return;
        }

        int i1 = 0, i2 = 1, i3 = 2;
        for (size_t t = 0; t < n; ++t) {
            i1 = (n/2 + t + n - 1) % n;
            i2 = (n/2 + t) % n;
            i3 = (n/2 + t + 1) % n;

            Vec3 d1 = vecs[i3] - vecs[i2];
            Vec3 d2 = vecs[i1] - vecs[i2];
            float cp = d1.cross(d2).z;
            bool convex = (cp == 0) || ((cp > 0) != orient);
            if (convex) break;
        }

        Vec3 v1 = vecs[i1], v2 = vecs[i2], v3 = vecs[i3];
        // ear clipping check
        float a = v1.y - v3.y;
        float b = v3.x - v1.x;
        float c = (v1.x - v3.x)*v1.y + (v3.y - v1.y)*v1.x;

        int max_k = -1;
        float max_dist = 0;

        for (size_t k = 0; k < n; ++k) {
            if (k == (size_t)i1 || k == (size_t)i2 || k == (size_t)i3) continue;
            if (pointInTriangle(vecs[k], v1, v2, v3)) {
                float dist = std::abs(a*vecs[k].x + b*vecs[k].y + c);
                if (max_k == -1 || dist > max_dist) {
                    max_dist = dist;
                    max_k = k;
                }
            }
        }

        if (max_k == -1) {
            out.insert(out.end(), {idxs[i1], idxs[i2], idxs[i3]});
            vecs.erase(vecs.begin() + i2);
            idxs.erase(idxs.begin() + i2);
            triangularizeRecurse(vecs, idxs, orient, out);

        } else {
            // split polygon
            std::vector<Vec3> vA, vB;
            std::vector<int> iA, iB;
            bool side = false;
            for(size_t r = 0; r < n; ++r) {
                if (r == (size_t)i2 || r == (size_t)max_k) {
                    vA.push_back(vecs[r]); iA.push_back(idxs[r]);
                    vB.push_back(vecs[r]); iB.push_back(idxs[r]);
                    side = !side;
                } else if (side) {
                    vA.push_back(vecs[r]); iA.push_back(idxs[r]);
                } else {
                    vB.push_back(vecs[r]); iB.push_back(idxs[r]);
                }
            }
            triangularizeRecurse(vA, iA, orient, out);
            triangularizeRecurse(vB, iB, orient, out);
        }
    }
}

// model methods

int Model::getMaterialIdx(const std::string& name) const {
    for (size_t i = 0; i < materials.size(); ++i) {
        if (materials[i].name == name) return i;
    }
    return -1;
}

Model Model::loadFromObj(const std::string& filename, bool use_colors) {
    Model m;
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "ERROR: Failed to open " << filename << "\n";
        return m;
    }

    std::string line, token;
    int current_mat = -1;

    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') continue;
        std::stringstream ss(line);
        ss >> token;

        if (token == "v") {
            float x, y, z;
            ss >> x >> y >> z;
            m.vertices.emplace_back(x, y, z);

        } else if (token == "f") {
            std::vector<int> f_idxs;
            std::string segment;
            while (ss >> segment) {
                size_t slash = segment.find('/');
                int idx = std::stoi(segment.substr(0, slash));
                if (idx < 0) idx += m.vertices.size();
                else idx -= 1;
                f_idxs.push_back(idx);
            }

            if (f_idxs.size() == 3) {
                m.faces.push_back({
                    {
                        (int)f_idxs[0], 
                        (int)f_idxs[1], 
                        (int)f_idxs[2]
                    }, 
                    current_mat
                });

            } else if (f_idxs.size() > 3) {
                // prep for triangularization
                std::vector<Vec3> poly_vecs;
                std::vector<int> poly_idxs = f_idxs;
                
                // calculate face normal to project to 2D
                Vec3 d1 = m.vertices[f_idxs[1]] - m.vertices[f_idxs[0]];
                Vec3 d2 = m.vertices[f_idxs[2]] - m.vertices[f_idxs[1]];
                Vec3 norm = d1.cross(d2).normalize();
                Vec3 perp = norm.cross(d1).normalize();
                Vec3 dir1 = d1.normalize();
                
                for(int idx : f_idxs) {
                    Vec3 v = m.vertices[idx];
                    poly_vecs.emplace_back(dir1.dot(v), perp.dot(v), 0);
                }

                // determine orientation
                float area = 0;
                for (size_t i = 0; i < poly_vecs.size(); ++i) {
                    Vec3 v1 = poly_vecs[i];
                    Vec3 v2 = poly_vecs[(i + 1) % poly_vecs.size()];
                    area += (v2.x - v1.x) * (v2.y + v1.y);
                }
                
                std::vector<int> tri_indices;
                triangularizeRecurse(poly_vecs, poly_idxs, area >= 0, tri_indices);
                
                for(size_t i=0; i < tri_indices.size(); i+=3) {
                    m.faces.push_back({
                        {
                            tri_indices[i], 
                            tri_indices[i+1], 
                            tri_indices[i+2]
                        }, 
                        current_mat
                    });
                }
            }

        } else if (use_colors && token == "mtllib") {
            std::string mtl_file;
            ss >> mtl_file;
            
            // manual path joining
            std::string mtl_path;
            size_t last_slash_idx = filename.rfind('/');
            if (std::string::npos == last_slash_idx) {
                last_slash_idx = filename.rfind('\\');
            }
            if (std::string::npos != last_slash_idx) {
                mtl_path = filename.substr(0, last_slash_idx + 1) + mtl_file;
            } else {
                mtl_path = mtl_file;
            }
            
            std::ifstream mtl(mtl_path);
            if (mtl.is_open()) {
                std::string mline, mtok;
                while(std::getline(mtl, mline)) {
                    std::stringstream mss(mline);
                    mss >> mtok;
                    if(mtok == "newmtl") {
                        std::string name; mss >> name;
                        m.materials.push_back({name});
                    } else if (mtok == "Kd" && !m.materials.empty()) {
                        mss >> m.materials.back().kd[0] >> m.materials.back().kd[1] >> m.materials.back().kd[2];
                    }
                }
            }

        } else if (use_colors && token == "usemtl") {
            std::string mat_name;
            ss >> mat_name;
            current_mat = m.getMaterialIdx(mat_name);
        }
    }
    return m;
}

Model Model::loadFromStl(const std::string& filename) {
    Model m;
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return m;

    char header[80];
    file.read(header, 80);
    std::string headerStr(header, 5);
    
    if (headerStr == "solid") {
        // ASCII STL
        file.close(); 
        file.open(filename);
        std::string line, token;
        while(std::getline(file, line)) {
            std::stringstream ss(line);
            ss >> token;
            if (token == "vertex") {
                float x, y, z;
                ss >> x >> y >> z;
                m.vertices.emplace_back(x, z, y);
            }
        }
        
        for(size_t i = 0; i < m.vertices.size(); i += 3) {
            m.faces.push_back({
                {
                    (int)i, 
                    (int)i+2, 
                    (int)i+1
                }, 
                -1
            });
        }

    } else {
        // binary STL
        uint32_t count;
        file.read(reinterpret_cast<char*>(&count), 4);

        for(uint32_t i=0; i<count; ++i) {
            float buffer[12]; // normal(3) + 3 verts(9)
            file.read(reinterpret_cast<char*>(buffer), 50); // 12 floats + 2 bytes spacer

            for(int v=0; v<3; ++v) {
                m.vertices.emplace_back(buffer[3 + v*3], buffer[5 + v*3], buffer[4 + v*3]); // swap Y/Z
            }
            m.faces.push_back({
                {
                    (int)m.vertices.size()-3, 
                    (int)m.vertices.size()-1, 
                    (int)m.vertices.size()-2
                },
                -1
            });
        }
    }
    return m;
}

void Model::normalize() {
    if (vertices.empty()) return;
    
    Vec3 minV = vertices[0], maxV = vertices[0];
    for (auto& v : vertices) {
        minV.x = std::min(minV.x, v.x);
        minV.y = std::min(minV.y, v.y);
        minV.z = std::min(minV.z, v.z);

        maxV.x = std::max(maxV.x, v.x);
        maxV.y = std::max(maxV.y, v.y);
        maxV.z = std::max(maxV.z, v.z);
    }
    
    Vec3 center = (minV + maxV) * 0.5f;
    float max_dist = 0;
    
    for(auto& v : vertices) {
        v = v - center;
        float d = v.mag();
        if(d > max_dist) max_dist = d;
    }
    
    float scale = (max_dist == 0) ? 1.0f : 1.0f / max_dist;
    for(auto& v : vertices) v = v * scale;
}

void Model::invertTriangles() {
    for(auto& f : faces) std::swap(f.idxs[1], f.idxs[2]);
}

void Model::transform(int axis1, int axis2, int axis3, bool invX, bool invY, bool invZ) {
    for(auto& v : vertices) {
        float old[3] = {v.x, v.y, v.z};
        v.x = old[axis1] * (invX ? -1 : 1);
        v.y = old[axis2] * (invY ? -1 : 1);
        v.z = old[axis3] * (invZ ? -1 : 1);
    }
}