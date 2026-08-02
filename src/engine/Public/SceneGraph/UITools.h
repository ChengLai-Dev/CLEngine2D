#pragma once

#include "Math/Vec3.h"

class Node;
class Widget;

namespace UITools {
    // 设计时拾取：后画先命中，忽略 touchEnabled 与 enabled，仅按 visible + 几何相交。
    // 与 UISystem::HitTestScene（运行时触摸交互）语义分离；root 为当前编辑场景根节点。
    Widget* HitTestDesign(Node* root, const Vec3& worldPoint);
}