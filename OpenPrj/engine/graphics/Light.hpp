#pragma once

#include "../core/Math.hpp"

namespace engine {

enum class LightType {
    Directional,
    Point,
    Spot,
};

struct Light {
    LightType type;
    Vec3f position;
    Vec3f direction;
    Vec3f color;
    float intensity;
    float range;

    Light(LightType t = LightType::Directional)
        : type(t), position(0, 1, 0), direction(0, -1, 0),
          color(1, 1, 1), intensity(1.0f), range(100.0f) {}

    static Light CreateDirectional(const Vec3f& dir, const Vec3f& col, float intensity = 1.0f) {
        Light light(LightType::Directional);
        light.direction = dir.Normalize();
        light.color = col;
        light.intensity = intensity;
        return light;
    }

    static Light CreatePoint(const Vec3f& pos, const Vec3f& col, float range = 10.0f, float intensity = 1.0f) {
        Light light(LightType::Point);
        light.position = pos;
        light.color = col;
        light.range = range;
        light.intensity = intensity;
        return light;
    }

    static Light CreateSpot(const Vec3f& pos, const Vec3f& dir, const Vec3f& col, float range = 10.0f, float intensity = 1.0f) {
        Light light(LightType::Spot);
        light.position = pos;
        light.direction = dir.Normalize();
        light.color = col;
        light.range = range;
        light.intensity = intensity;
        return light;
    }
};

}
