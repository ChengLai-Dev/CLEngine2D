#pragma once

#include "Math/Vec3.h"
#include <memory>

class Texture;

struct SpriteComponent {
    Vec3 Position = Vec3(0.0f, 0.0f, 0.0f);
    Vec3 Size = Vec3(1.0f, 1.0f, 1.0f);
    float Rotation = 0.0f;
    std::shared_ptr<Texture> Texture = nullptr;
    float Color[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float TexOffsetX = 0.0f;
    float TexOffsetY = 0.0f;
    float TexScaleX = 1.0f;
    float TexScaleY = 1.0f;
};
