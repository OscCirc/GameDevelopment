#pragma once
#include "Vec2.h"
#include <vector>
#include <random>

// -----------------------------------------------------------------------
// Boid 参数（可统一调节）
// -----------------------------------------------------------------------
struct BoidConfig {
    float maxSpeed        = 4.0f;   // 最大速度
    float minSpeed        = 1.5f;   // 最低速度（防止静止）
    float maxForce        = 0.3f;   // 每帧最大转向力

    float separationRadius = 25.0f; // 分离感知半径
    float alignRadius      = 50.0f; // 对齐感知半径
    float cohesionRadius   = 50.0f; // 聚合感知半径

    float sepWeight     = 1.5f;     // 分离权重
    float aliWeight     = 1.0f;     // 对齐权重
    float cohWeight     = 1.0f;     // 聚合权重
    float wanderWeight  = 0.08f;    // 随机游走权重（打破平衡态）

    float worldWidth  = 800.f;
    float worldHeight = 600.f;
};

// -----------------------------------------------------------------------
// Boid 个体
// -----------------------------------------------------------------------
class Boid {
public:
    Vec2 pos;
    Vec2 vel;

    Boid(float x, float y, float vx, float vy)
        : pos(x, y), vel(vx, vy) {}

    // 用邻居列表更新加速度并积分位置
    void update(const std::vector<Boid>& flock, const BoidConfig& cfg) {
        Vec2 sep = separation(flock, cfg);
        Vec2 ali = alignment(flock, cfg);
        Vec2 coh = cohesion(flock, cfg);
        Vec2 wan = wander(cfg);

        Vec2 accel = sep * cfg.sepWeight
                   + ali * cfg.aliWeight
                   + coh * cfg.cohWeight
                   + wan * cfg.wanderWeight;

        vel = (vel + accel).limited(cfg.maxSpeed);

        // 保证最低速度，防止群体"冻结"
        float spd = vel.length();
        if (spd < cfg.minSpeed && spd > 1e-6f)
            vel = vel * (cfg.minSpeed / spd);

        pos += vel;
        wrapEdges(cfg);
    }

    // 朝向速度方向的角度（度），用于显示
    float heading() const {
        return std::atan2(vel.y, vel.x) * 180.f / 3.14159265f;
    }

private:
    // ------------------------------------------------------------------
    // 规则1：分离 —— 避免与过近邻居重叠
    // ------------------------------------------------------------------
    Vec2 separation(const std::vector<Boid>& flock, const BoidConfig& cfg) const {
        Vec2  steer;
        int   count = 0;
        float r2    = cfg.separationRadius * cfg.separationRadius;

        for (const Boid& other : flock) {
            if (&other == this) continue;
            Vec2  diff   = pos - other.pos;
            float distSq = diff.lengthSq();
            if (distSq > 0.f && distSq < r2) {
                // 越近排斥越强（按距离反比加权）
                steer += diff.normalized() / std::sqrt(distSq);
                ++count;
            }
        }
        if (count > 0) {
            steer = steer / static_cast<float>(count);
            // normalize 到 maxSpeed 再求转向力（Reynolds 标准做法）
            steer = steer.normalized() * cfg.maxSpeed;
            steer = (steer - vel).limited(cfg.maxForce);
        }
        return steer;
    }

    // ------------------------------------------------------------------
    // 规则2：对齐 —— 与邻居速度方向趋同
    // ------------------------------------------------------------------
    Vec2 alignment(const std::vector<Boid>& flock, const BoidConfig& cfg) const {
        Vec2  sum;
        int   count = 0;
        float r2    = cfg.alignRadius * cfg.alignRadius;

        for (const Boid& other : flock) {
            if (&other == this) continue;
            if ((pos - other.pos).lengthSq() < r2) {
                sum += other.vel;
                ++count;
            }
        }
        if (count > 0) {
            // desired = 邻居平均速度方向，缩放到 maxSpeed
            Vec2 desired = (sum / static_cast<float>(count)).normalized() * cfg.maxSpeed;
            return (desired - vel).limited(cfg.maxForce);
        }
        return {};
    }

    // ------------------------------------------------------------------
    // 规则3：聚合 —— 向邻居质心靠拢
    // ------------------------------------------------------------------
    Vec2 cohesion(const std::vector<Boid>& flock, const BoidConfig& cfg) const {
        Vec2  center;
        int   count = 0;
        float r2    = cfg.cohesionRadius * cfg.cohesionRadius;

        for (const Boid& other : flock) {
            if (&other == this) continue;
            if ((pos - other.pos).lengthSq() < r2) {
                center += other.pos;
                ++count;
            }
        }
        if (count > 0) {
            center = center / static_cast<float>(count);
            // desired = 指向质心方向，以 maxSpeed 全速飞去
            Vec2 desired = (center - pos).normalized() * cfg.maxSpeed;
            return (desired - vel).limited(cfg.maxForce);
        }
        return {};
    }

    // ------------------------------------------------------------------
    // 随机游走 —— 给速度方向施加微小随机扰动，防止群体冻结
    // ------------------------------------------------------------------
    Vec2 wander(const BoidConfig& cfg) const {
        static thread_local std::mt19937 rng(std::random_device{}());
        static thread_local std::uniform_real_distribution<float> dist(-1.f, 1.f);
        return Vec2(dist(rng), dist(rng)).limited(cfg.maxForce);
    }

    // 环绕边界（穿墙）
    void wrapEdges(const BoidConfig& cfg) {
        if (pos.x < 0)             pos.x += cfg.worldWidth;
        if (pos.x > cfg.worldWidth)  pos.x -= cfg.worldWidth;
        if (pos.y < 0)             pos.y += cfg.worldHeight;
        if (pos.y > cfg.worldHeight) pos.y -= cfg.worldHeight;
    }
};
