#pragma once

#include "Render/Renderer.h"

#include <vector>

struct DBSlotVertexData {
    std::vector<QuadVertex> vertices;
    std::vector<unsigned int> indices;
    Texture* texture = nullptr;
};
