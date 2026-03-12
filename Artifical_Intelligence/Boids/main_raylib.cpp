// =======================================================================
// Boids — Raylib 图形版
// 效果：三角形 boid，按朝向 HSV 染色，拖影轨迹，实时参数显示
// 依赖：raylib (https://www.raylib.com)
// =======================================================================
#include "Flock.h"
#include "raylib.h"
#include <deque>
#include <vector>
#include <cmath>

// -----------------------------------------------------------------------
// 工具：将朝向角(deg)映射为 HSV 颜色
// -----------------------------------------------------------------------
static Color headingToColor(float deg) {
    // 角度 [-180, 180] → hue [0, 360]
    float hue = deg + 180.f;               // [0, 360]
    return ColorFromHSV(hue, 0.85f, 1.0f);
}

// -----------------------------------------------------------------------
// 绘制单个三角形 boid
//   pos      : 世界坐标
//   heading  : 速度方向(deg)
//   size     : 三角形"长轴"半径(像素)
//   col      : 颜色
// -----------------------------------------------------------------------
static void drawBoid(Vector2 pos, float headingDeg, float size, Color col) {
    float rad  = headingDeg * DEG2RAD;
    float rOff = 150.f * DEG2RAD; // 底边两顶点偏转角

    // 尖端（朝速度方向）
    Vector2 tip = {
        pos.x + std::cos(rad)            * size,
        pos.y + std::sin(rad)            * size
    };
    // 左翼
    Vector2 left = {
        pos.x + std::cos(rad + rOff) * (size * 0.5f),
        pos.y + std::sin(rad + rOff) * (size * 0.5f)
    };
    // 右翼
    Vector2 right = {
        pos.x + std::cos(rad - rOff) * (size * 0.5f),
        pos.y + std::sin(rad - rOff) * (size * 0.5f)
    };

    DrawTriangle(tip, left, right, col);
    // 描边增加清晰度
    DrawTriangleLines(tip, left, right, Fade(WHITE, 0.3f));
}

int main() {
    // ---- 窗口 & 世界参数 ----
    const int   WIN_W      = 1200;
    const int   WIN_H      = 800;
    const int   BOID_COUNT = 150;
    const float BOID_SIZE  = 8.f;
    const int   TRAIL_LEN  = 20;    // 尾迹帧数

    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(WIN_W, WIN_H, "Boids Simulation — Raylib");
    SetTargetFPS(60);

    // ---- 初始化群体 ----
    Flock flock(BOID_COUNT);
    flock.cfg.worldWidth      = static_cast<float>(WIN_W);
    flock.cfg.worldHeight     = static_cast<float>(WIN_H);
    flock.cfg.maxSpeed        = 6.0f;
    flock.cfg.minSpeed        = 2.5f;
    flock.cfg.maxForce        = 0.25f;
    flock.cfg.separationRadius = 30.f;
    flock.cfg.alignRadius      = 70.f;
    flock.cfg.cohesionRadius   = 70.f;
    flock.cfg.sepWeight        = 1.5f;
    flock.cfg.aliWeight        = 1.2f;
    flock.cfg.cohWeight        = 0.8f;
    flock.cfg.wanderWeight     = 0.06f;

    // 每个 boid 的历史轨迹
    std::vector<std::deque<Vector2>> trails(BOID_COUNT);

    // ---- 主循环 ----
    while (!WindowShouldClose()) {

        // -- 更新轨迹 --
        const auto& boids = flock.getBoids();
        for (int i = 0; i < (int)boids.size(); ++i) {
            trails[i].push_back({boids[i].pos.x, boids[i].pos.y});
            if ((int)trails[i].size() > TRAIL_LEN)
                trails[i].pop_front();
        }

        flock.tick();

        // -- 键盘实时调参 --
        if (IsKeyDown(KEY_Q)) flock.cfg.sepWeight = std::max(0.f, flock.cfg.sepWeight - 0.02f);
        if (IsKeyDown(KEY_W)) flock.cfg.sepWeight += 0.02f;
        if (IsKeyDown(KEY_A)) flock.cfg.aliWeight = std::max(0.f, flock.cfg.aliWeight - 0.02f);
        if (IsKeyDown(KEY_S)) flock.cfg.aliWeight += 0.02f;
        if (IsKeyDown(KEY_Z)) flock.cfg.cohWeight = std::max(0.f, flock.cfg.cohWeight - 0.02f);
        if (IsKeyDown(KEY_X)) flock.cfg.cohWeight += 0.02f;

        // -- 绘制 --
        BeginDrawing();
        ClearBackground({10, 10, 20, 255}); // 深蓝黑背景

        // 画尾迹（跳过跨边界的线段）
        const float WRAP_THRESHOLD_SQ = (flock.cfg.worldWidth  * 0.3f) *
                                        (flock.cfg.worldWidth  * 0.3f);
        for (int i = 0; i < (int)boids.size(); ++i) {
            Color baseCol = headingToColor(boids[i].heading());
            int   tlen    = static_cast<int>(trails[i].size());
            for (int t = 1; t < tlen; ++t) {
                float dx = trails[i][t].x - trails[i][t-1].x;
                float dy = trails[i][t].y - trails[i][t-1].y;
                if (dx*dx + dy*dy > WRAP_THRESHOLD_SQ) continue; // 跨边界，跳过
                float alpha = static_cast<float>(t) / TRAIL_LEN;
                Color tc    = Fade(baseCol, alpha * 0.35f);
                float thick = 1.5f * alpha;
                DrawLineEx(trails[i][t - 1], trails[i][t], thick, tc);
            }
        }

        // 画 boid 本体
        for (int i = 0; i < (int)boids.size(); ++i) {
            Vector2 p   = {boids[i].pos.x, boids[i].pos.y};
            Color   col = headingToColor(boids[i].heading());
            drawBoid(p, boids[i].heading(), BOID_SIZE, col);
        }

        // HUD
        DrawFPS(10, 10);
        DrawText(TextFormat("Boids: %d", (int)flock.size()), 10, 35, 18, RAYWHITE);
        DrawText(TextFormat("[Q/W] Sep: %.2f", flock.cfg.sepWeight), 10, 60,  18, LIGHTGRAY);
        DrawText(TextFormat("[A/S] Ali: %.2f", flock.cfg.aliWeight), 10, 82,  18, LIGHTGRAY);
        DrawText(TextFormat("[Z/X] Coh: %.2f", flock.cfg.cohWeight), 10, 104, 18, LIGHTGRAY);

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
