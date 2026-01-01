#include "model.hpp"
#include "surface.hpp"
#include <ncurses.h>
#include <unistd.h>
#include <iostream>
#include <chrono>
#include <cstring>
#include <thread>
#include <algorithm>
#include <cmath>

struct Config {
    std::string input_file;
    int w = 0, h = 0;
    int fps = 20;
    float zoom = 100.0f;
    bool interactive = false;
    bool color = false;
    std::string chars = ".,':;!+*=#$@";
    int axes[3] = {0, 1, 2};
    bool inv[3] = {false, false, false};
};

char getLumChar(const Vec3& norm, const Vec3& light, const std::string& chars) {
    float sim = norm.dot(light) * 0.5f + 0.5f;
    size_t idx = std::clamp((size_t)std::round((chars.size() - 1) * sim), (size_t)0, chars.size() - 1);
    return chars[idx];
}

Vec3 mapToSurface(const Vec3& v, const Surface& surf, float lw, float lh, float zoom) {
    return {
        0.5f * lw + 0.5f * v.x * zoom,
        0.5f * lh - 0.5f * v.y * zoom,
        0.5f + 0.5f * v.z * zoom
    };
}

void run(Model& model, Config& cfg) {
    initscr();
    noecho();
    curs_set(0);
    timeout(0);
    keypad(stdscr, TRUE);
    
    if (cfg.w == 0) getmaxyx(stdscr, cfg.h, cfg.w);
    
    // initialize colors
    if (cfg.color) {
        start_color();
        if (can_change_color()) {
            for(size_t i=0; i < model.materials.size(); ++i) {
                auto& m = model.materials[i];
                // scale 0-1 float to 0-1000 short for ncurses
                init_color(i+1, (short)(m.kd[0]*1000), (short)(m.kd[1]*1000), (short)(m.kd[2]*1000));
                init_pair(i+1, i+1, 0);
            }
        }
    }

    // aspect ratio correction for characters
    float logical_h = 1.0f;
    float logical_w = (float)cfg.w / (cfg.h * 1.8f); 
    
    Surface surface(cfg.w, cfg.h, logical_w, logical_h);
    
    // state variables
    float az = 0, al = 0;
    float zoom = cfg.zoom / 100.0f;
    bool running = true;
    Vec3 light = Vec3(1, -1, 0).normalize();
    
    // animation constants
    const float PI = 3.14159265359f;
    const float GOLDEN_RATIO = 1.6180339887f;
    const float az_speed = 2.0f;
    const float al_speed = GOLDEN_RATIO * 0.25f;

    auto start_time = std::chrono::steady_clock::now();
    auto next_frame = start_time;
    int frame_us = 1000000 / cfg.fps;

    while(running) {
        auto now = std::chrono::steady_clock::now();
        
        // rotation logic
        if (!cfg.interactive) {
            std::chrono::duration<float> elapsed = now - start_time;
            float t = elapsed.count();
            
            az = az_speed * t;
            // oscillate altitude slightly for 3D effect
            al = 0.125f * PI * (1.0f - std::sin(al_speed * t)); 
        }

        // drawing
        surface.clear();

        float cos_az = std::cos(az), sin_az = std::sin(az);
        float cos_al = std::cos(-al), sin_al = std::sin(-al);

        for (const auto& face : model.faces) {
            Triangle t = {
                model.vertices[face.idxs[0]], 
                model.vertices[face.idxs[1]], 
                model.vertices[face.idxs[2]]
            };
            
            // rotate
            auto transform = [&](Vec3 v) {
                v = v.rotateY(cos_az, sin_az);
                v = v.rotateX(cos_al, sin_al);
                return v;
            };

            t.p1 = transform(t.p1);
            t.p2 = transform(t.p2);
            t.p3 = transform(t.p3);

            // lighting (calculate normal after rotation)
            Vec3 normal = (t.p2 - t.p1).cross(t.p3 - t.p1).normalize();
            char c = getLumChar(normal * -1.0f, light, cfg.chars);

            // map to screen surface
            t.p1 = mapToSurface(t.p1, surface, logical_w, logical_h, zoom);
            t.p2 = mapToSurface(t.p2, surface, logical_w, logical_h, zoom);
            t.p3 = mapToSurface(t.p3, surface, logical_w, logical_h, zoom);
            
            surface.drawTriangle(t, c, face.material_idx);
        }

        surface.printNCurses(cfg.color);
        refresh();

        // input handling
        int ch = getch();
        if (ch == 'q') running = false;
        
        if (cfg.interactive) {
            if (ch == KEY_LEFT) az += 0.1f;
            if (ch == KEY_RIGHT) az -= 0.1f;
            if (ch == KEY_UP) al += 0.1f;
            if (ch == KEY_DOWN) al -= 0.1f;
        }

        if (ch == '+') zoom *= 1.1f;
        if (ch == '-') zoom *= 0.9f;
        
        // timing
        next_frame += std::chrono::microseconds(frame_us);
        std::this_thread::sleep_until(next_frame);
    }

    endwin();
}

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " [options] file.obj\n";
        std::cerr << "Options:\n";
        std::cerr << "  -i, --interactive   Manual control (Arrow keys)\n";
        std::cerr << "  -c, --color         Enable colors (if supported)\n";
        std::cerr << "  -z, --zoom <num>    Zoom level (default 100)\n";
        return 1;
    }

    Config cfg;
    for(int i = 1; i < argc; ++i) {
        std::string arg = argv[i];
        if (arg == "--color" || arg == "-c") cfg.color = true;
        else if (arg == "--interactive" || arg == "-i") cfg.interactive = true;
        else if ((arg=="--zoom" || arg=="-z") && i+1 < argc) cfg.zoom = std::stof(argv[++i]);
        else if (arg[0] != '-') cfg.input_file = arg;
    }

    if (cfg.input_file.empty()) return 1;

    Model model;
    if (cfg.input_file.find(".obj") != std::string::npos) {
        model = Model::loadFromObj(cfg.input_file, cfg.color);
        model.invertTriangles(); // fix winding order
        model.transform(0, 1, 2, false, false, true); // invert z for obj standard
    } else {
        model = Model::loadFromStl(cfg.input_file);
    }
    
    if (model.vertices.empty()) {
        std::cerr << "Error: No vertices loaded.\n";
        return 1;
    }

    model.normalize();
    run(model, cfg);
    return 0;
}