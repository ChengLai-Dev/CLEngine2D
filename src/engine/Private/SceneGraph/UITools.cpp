#include "SceneGraph/UITools.h"
#include "SceneGraph/Node.h"
#include "SceneGraph/Widget.h"

namespace UITools {
namespace {

Widget* HitTestDesignRecursive(Node* node, const Vec3& worldPoint) {
    if (!node || !node->IsVisible()) return nullptr;

    for (size_t i = node->GetChildCount(); i > 0; --i) {
        Widget* found = HitTestDesignRecursive(node->GetChild(i - 1), worldPoint);
        if (found) return found;
    }

    Widget* widget = dynamic_cast<Widget*>(node);
    if (widget && widget->HitTestGeometry(worldPoint)) {
        return widget;
    }

    return nullptr;
}

}  // namespace

Widget* HitTestDesign(Node* root, const Vec3& worldPoint) {
    return HitTestDesignRecursive(root, worldPoint);
}

}  // namespace UITools