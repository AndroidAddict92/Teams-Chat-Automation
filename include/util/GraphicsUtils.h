#pragma once
#include <imgui.h>

namespace GraphicsUtil {
    void DrawGradientRect(
        ImDrawList* dl,
        const ImVec2& a,
        const ImVec2& b,
        ImU32 colA,
        ImU32 colB
    );
}