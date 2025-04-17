#include <util/GraphicsUtils.h>

void GraphicsUtil::DrawGradientRect(
    ImDrawList* dl,
    const ImVec2& a,
    const ImVec2& b,
    ImU32 colA,
    ImU32 colB
) {
    dl->AddRectFilledMultiColor(a, b, colA, colA, colB, colB);
}