#include <pch.h>

#include "CustomWidgets.h"

#include <imgui.h>
#include <imgui_internal.h>

void verticalProgressBar(float fraction, const ImVec2& size_arg){

    ImGuiWindow* window = ImGui::GetCurrentWindow();
    if (window->SkipItems) return;

    ImGuiContext& g = *GImGui;
    const ImGuiStyle& style = g.Style;

    ImVec2 pos = window->DC.CursorPos;
    //ImVec2 size = ImGui::CalcItemSize(size_arg, ImGui::CalcItemWidth(), g.FontSize + style.FramePadding.y * 2.0f);
    ImVec2 size = size_arg;
    ImVec2 rectMax(pos.x + size.x, pos.y + size.y);
    ImRect bb(pos, rectMax);
    ImGui::ItemSize(size, style.FramePadding.y);
    if (!ImGui::ItemAdd(bb, 0)) return;

    // Render
    fraction = ImSaturate(fraction);
    ImGui::RenderFrame(bb.Min, bb.Max, ImGui::GetColorU32(ImGuiCol_FrameBg), true, style.FrameRounding);
    bb.Expand(ImVec2(-style.FrameBorderSize, -style.FrameBorderSize));
    ImVec2 progressBarMin(pos.x, pos.y + size.y * (1.0 - fraction));
    ImGui::RenderFrame(progressBarMin, rectMax, ImGui::GetColorU32(ImGuiCol_PlotHistogram), false, style.FrameRounding);
}



bool buttonCross(const char* id, float size) {
    glm::vec2 center;
    float cross_extent;
    bool ret;
    if (size == 0) {
        ret = ImGui::Button(id, glm::vec2(ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0));
    }
    else {
        ret = ImGui::Button(id, glm::vec2(size));
    }
    center = ImGui::GetItemRectMin();
    center += glm::vec2(ImGui::GetItemRectSize().x * 0.5);
    cross_extent = ImGui::GetTextLineHeight() * 0.5f * 0.7071f - 1.0f;
    ImGui::GetWindowDrawList()->AddLine(center + glm::vec2(+cross_extent, +cross_extent), center + glm::vec2(-cross_extent, -cross_extent), ImColor(glm::vec4(1.0, 1.0, 1.0, 1.0)), ImGui::GetTextLineHeight() * 0.15);
    ImGui::GetWindowDrawList()->AddLine(center + glm::vec2(+cross_extent, -cross_extent), center + glm::vec2(-cross_extent, +cross_extent), ImColor(glm::vec4(1.0, 1.0, 1.0, 1.0)), ImGui::GetTextLineHeight() * 0.15);
    return ret;
}