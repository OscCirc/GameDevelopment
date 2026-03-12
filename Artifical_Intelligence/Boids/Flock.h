#pragma once
#include "Boid.h"
#include <vector>
#include <random>
#include <ctime>

// -----------------------------------------------------------------------
// Flock —— 管理所有 Boid，每帧统一更新
// -----------------------------------------------------------------------
class Flock {
public:
    BoidConfig cfg;

    explicit Flock(int count) {
        std::mt19937 rng(static_cast<unsigned>(std::time(nullptr)));
        std::uniform_real_distribution<float> rx(0.f, cfg.worldWidth);
        std::uniform_real_distribution<float> ry(0.f, cfg.worldHeight);
        std::uniform_real_distribution<float> rv(-cfg.maxSpeed, cfg.maxSpeed);

        boids.reserve(count);
        for (int i = 0; i < count; ++i)
            boids.emplace_back(rx(rng), ry(rng), rv(rng), rv(rng));
    }

    // 每帧调用：用上一帧状态计算新状态（避免更新顺序影响结果）
    void tick() {
        std::vector<Boid> snapshot = boids; // 快照
        for (Boid& b : boids)
            b.update(snapshot, cfg);
    }

    const std::vector<Boid>& getBoids() const { return boids; }
    std::size_t size() const { return boids.size(); }

private:
    std::vector<Boid> boids;
};
