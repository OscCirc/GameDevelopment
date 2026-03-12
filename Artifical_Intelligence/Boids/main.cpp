// =======================================================================
// Boids — 经典鸟群算法演示 (控制台版)
// 规则：Separation · Alignment · Cohesion
// =======================================================================
#include "Flock.h"
#include <iostream>
#include <iomanip>
#include <string>
#include <thread>
#include <chrono>

#ifdef _WIN32
#  include <windows.h>
static void clearScreen() { system("cls"); }
#else
static void clearScreen() { system("clear"); }
#endif

// 将世界坐标映射到字符网格并绘制
static void render(const Flock& flock, int cols, int rows) {
    // 用空格填满画布
    std::vector<std::string> canvas(rows, std::string(cols, ' '));

    for (const Boid& b : flock.getBoids()) {
        int cx = static_cast<int>(b.pos.x / flock.cfg.worldWidth  * cols);
        int cy = static_cast<int>(b.pos.y / flock.cfg.worldHeight * rows);
        cx = std::max(0, std::min(cols - 1, cx));
        cy = std::max(0, std::min(rows - 1, cy));

        // 根据速度方向选择字符
        float h = b.heading();
        char  c;
        if      (h >= -22.5f  && h <  22.5f)  c = '>';
        else if (h >=  22.5f  && h <  67.5f)  c = 'v';  // 右下
        else if (h >=  67.5f  && h < 112.5f)  c = 'v';
        else if (h >= 112.5f  && h < 157.5f)  c = '<';  // 左下
        else if (h >= 157.5f  || h < -157.5f) c = '<';
        else if (h >= -157.5f && h < -112.5f) c = '^';  // 左上
        else if (h >= -112.5f && h <  -67.5f) c = '^';
        else                                   c = '>';  // 右上

        canvas[cy][cx] = c;
    }

    // 打印边框 + 画布
    std::string border(cols + 2, '-');
    std::cout << border << '\n';
    for (const auto& row : canvas)
        std::cout << '|' << row << "|\n";
    std::cout << border << '\n';
    std::cout << "Boids: " << flock.size()
              << "  [sep=" << flock.cfg.sepWeight
              << " ali="   << flock.cfg.aliWeight
              << " coh="   << flock.cfg.cohWeight << "]\n";
}

int main() {
    const int BOID_COUNT = 80;
    const int COLS       = 100;
    const int ROWS       = 30;
    const int FRAMES     = 300;
    const int FRAME_MS   = 50;   // ~20 FPS

    Flock flock(BOID_COUNT);
    // 世界尺寸与字符网格对齐：1单位 = 1字符格，速度(4)每帧移动4格可见
    flock.cfg.worldWidth  = static_cast<float>(COLS);
    flock.cfg.worldHeight = static_cast<float>(ROWS);
    flock.cfg.maxSpeed        = 0.5f;
    flock.cfg.maxForce        = 0.04f;
    flock.cfg.separationRadius = 3.f;
    flock.cfg.alignRadius      = 6.f;
    flock.cfg.cohesionRadius   = 6.f;

    std::cout << "=== Boids Simulation ===\n";
    std::cout << "Rules: Separation | Alignment | Cohesion\n\n";

    for (int frame = 0; frame < FRAMES; ++frame) {
        clearScreen();
        std::cout << "Frame " << std::setw(4) << frame + 1 << " / " << FRAMES << "\n";
        render(flock, COLS, ROWS);

        flock.tick();
        std::this_thread::sleep_for(std::chrono::milliseconds(FRAME_MS));
    }

    std::cout << "\nSimulation finished.\n";
    return 0;
}
