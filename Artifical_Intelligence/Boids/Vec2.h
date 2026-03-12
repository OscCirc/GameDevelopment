#pragma once
#include <cmath>

struct Vec2 {
    float x, y;

    Vec2(float x = 0.f, float y = 0.f) : x(x), y(y) {}

    Vec2 operator+(const Vec2& o) const { return {x + o.x, y + o.y}; }
    Vec2 operator-(const Vec2& o) const { return {x - o.x, y - o.y}; }
    Vec2 operator*(float s)        const { return {x * s,   y * s};   }
    Vec2 operator/(float s)        const { return {x / s,   y / s};   }
    Vec2& operator+=(const Vec2& o) { x += o.x; y += o.y; return *this; }
    Vec2& operator-=(const Vec2& o) { x -= o.x; y -= o.y; return *this; }

    float lengthSq() const { return x * x + y * y; }
    float length()   const { return std::sqrt(lengthSq()); }

    // 返回单位向量；零向量时返回自身
    Vec2 normalized() const {
        float len = length();
        return len > 1e-6f ? (*this / len) : *this;
    }

    // 限制向量长度不超过 maxLen
    Vec2 limited(float maxLen) const {
        float len = length();
        return len > maxLen ? (*this / len * maxLen) : *this;
    }
};
