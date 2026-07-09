#pragma once

struct Color {
    float r, g, b, a;

    Color() : r(1.0f), g(1.0f), b(1.0f), a(1.0f) {}
    Color(float r, float g, float b, float a = 1.0f) : r(r), g(g), b(b), a(a) {}
};