#pragma once

struct HitRect {
    float x = 0.0f, y = 0.0f, w = 0.0f, h = 0.0f;
    bool Contains(float px, float py) const {
        return px >= x && px < x + w && py >= y && py < y + h;
    }
};
