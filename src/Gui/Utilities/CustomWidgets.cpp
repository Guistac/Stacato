#include <pch.h>

#include "CustomWidgets.h"

#include <imgui.h>
#include <imgui_internal.h>

void verticalProgressBar(float fraction, const ImVec2& size_arg){

    if (fraction > 1.0) fraction = 1.0;
    else if (fraction < 0.0) fraction = 0.0;
    else if (isnan(fraction)) fraction = 0.0;

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
    if (fraction == 0.0) return;
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

ToggleSwitch::ToggleSwitch(){
	toggleTime_seconds = ImGui::GetTime() - travelTime_seconds;
}

bool ToggleSwitch::draw(const char* ID, bool& data, const char* option1, const char* option2, ImVec2 size){
	
	bool toggled = false;
	if(ImGui::InvisibleButton(ID, size)) {
		toggleTime_seconds = ImGui::GetTime();
		toggled = true;
	}
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	glm::vec2 opt1Min = min;
	glm::vec2 opt1Max(max.x, min.y + size.y / 2.0);
	glm::vec2 opt2Min(min.x, min.y + size.y / 2.0);
	glm::vec2 opt2Max = max;
	
	ImDrawList* drawList = ImGui::GetWindowDrawList();
	
	drawList->AddRectFilled(min, max, ImGui::GetColorU32(ImGuiCol_MenuBarBg), 5.0);
	
	ImU32 selectionBoxColor = ImGui::GetColorU32(ImGuiCol_Button);
	if(ImGui::IsItemActive()) selectionBoxColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if(ImGui::IsItemHovered()) selectionBoxColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	
	float travelProgress = std::min(1.0, (ImGui::GetTime() - toggleTime_seconds) / travelTime_seconds);
	
	float selectionBoxHeight;
	if(toggled) selectionBoxHeight = data ? min.y : opt2Min.y;
	else selectionBoxHeight = data ? opt2Min.y - travelProgress * size.y / 2.0 : min.y + travelProgress * size.y / 2.0;
	
	ImVec2 selBoxMin = ImVec2(min.x, selectionBoxHeight);
	ImVec2 selBoxMax = ImVec2(max.x, selBoxMin.y + size.y / 2.0);
	
	drawList->AddRectFilled(selBoxMin, selBoxMax, selectionBoxColor, 5.0);
	 
	ImVec2 opt1TextSize = ImGui::CalcTextSize(option1);
	ImVec2 opt2TextSize = ImGui::CalcTextSize(option2);
	
	ImVec2 opt1TextPos = ImVec2(((opt1Min.x + opt1Max.x) / 2.0) - opt1TextSize.x / 2.0, ((opt1Min.y + opt1Max.y) / 2.0) - opt1TextSize.y / 2.0);
	ImVec2 opt2TextPos = ImVec2(((opt2Min.x + opt2Max.x) / 2.0) - opt2TextSize.x / 2.0, ((opt2Min.y + opt2Max.y) / 2.0) - opt2TextSize.y / 2.0);
	
	bool itemDisabled = ImGui::GetItemFlags() & ImGuiItemFlags_Disabled;
	
	ImU32 opt1TextColor = data && travelProgress == 1.0 && !itemDisabled ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
	ImU32 opt2TextColor = !data && travelProgress == 1.0 && !itemDisabled ? ImGui::GetColorU32(ImGuiCol_Text) : ImGui::GetColorU32(ImGuiCol_TextDisabled);
	
	drawList->AddText(opt1TextPos, opt1TextColor, option1);
	drawList->AddText(opt2TextPos, opt2TextColor, option2);
	
	
	
	return toggled;
}


namespace ListManagerWidget{
	Interaction draw(bool disableMoveUp, bool disableMoveDown, const char* ID, float buttonHeight){
		
		ImGui::PushID(ID);
		ImGui::BeginGroup();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
		
		if(buttonHeight == 0.0) buttonHeight = ImGui::GetTextLineHeight();
		ImVec2 buttonSize = ImVec2(buttonHeight, buttonHeight);
		float rounding = ImGui::GetStyle().FrameRounding;
		
		ImVec2 min, max;
		ImColor buttonColor;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImColor pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		
		bool deletePressed = ImGui::InvisibleButton("##delete", buttonSize);
		min = ImGui::GetItemRectMin();
		max = ImGui::GetItemRectMax();
		buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		drawList->AddRectFilled(min, max, buttonColor, rounding, ImDrawFlags_RoundCornersLeft);
		float crossSpacing = 0.3;
		float lineThickness = ImGui::GetTextLineHeight() * 0.1;
		drawList->AddLine(
						  ImVec2(min.x + buttonHeight * crossSpacing, min.y + buttonHeight * crossSpacing),
						  ImVec2(min.x + buttonHeight - buttonHeight * crossSpacing, min.y + buttonHeight - buttonHeight * crossSpacing),
						  pictogramColor, lineThickness);
		drawList->AddLine(
						  ImVec2(min.x + buttonHeight - buttonHeight * 0.3, min.y + buttonHeight * 0.3),
						  ImVec2(min.x + buttonHeight * 0.3, min.y + buttonHeight - buttonHeight * 0.3),
						  pictogramColor, lineThickness);

		float triangleSpacing = 0.3;
		
		ImGui::SameLine();
		if(disableMoveUp) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			pictogramColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
		}else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		bool moveUpPressed = ImGui::InvisibleButton("##moveup", buttonSize);
		min = ImGui::GetItemRectMin();
		max = ImGui::GetItemRectMax();
		buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		drawList->AddRectFilled(min, max, buttonColor);
		drawList->AddTriangleFilled(
									ImVec2(min.x + buttonHeight * 0.5f, min.y + buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight - buttonHeight * triangleSpacing, min.y + buttonHeight - buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight * triangleSpacing, min.y + buttonHeight - buttonHeight * triangleSpacing),
									pictogramColor);
		if(disableMoveUp) ImGui::PopItemFlag();
		
		ImGui::SameLine();
		if(disableMoveDown) {
			ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
			pictogramColor = ImGui::GetColorU32(ImGuiCol_TextDisabled);
		}else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		bool moveDownPressed = ImGui::InvisibleButton("##movedown", buttonSize);
		min = ImGui::GetItemRectMin();
		max = ImGui::GetItemRectMax();
		buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		drawList->AddRectFilled(min, max, buttonColor, rounding, ImDrawFlags_RoundCornersRight);
		drawList->AddTriangleFilled(
									ImVec2(min.x + buttonHeight * 0.5f, min.y + buttonHeight - buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight * triangleSpacing, min.y + buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight - buttonHeight * triangleSpacing, min.y + buttonHeight * triangleSpacing),
									pictogramColor);
		if(disableMoveDown) ImGui::PopItemFlag();
		
		ImGui::PopStyleVar();
		ImGui::EndGroup();
		ImGui::PopID();
		
		if(moveUpPressed) return Interaction::MOVE_UP;
		else if(moveDownPressed) return Interaction::MOVE_DOWN;
		else if(deletePressed) return Interaction::DELETE;
		else return Interaction::NONE;
	}
}

namespace BackgroundText{

	void draw(const char* text){
		draw(text, ImVec2(0.0, 0.0));
	}
	void draw(const char* text, ImVec2 size){
		glm::vec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
		glm::vec4 textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
		draw(text, size, buttonColor, textColor);
	}
	void draw(const char* text, ImVec4 backgroundColor){
		draw(text, ImVec2(0.0, 0.0), backgroundColor);
	}
	void draw(const char* text, ImVec2 size, ImVec4 backgroundColor){
		glm::vec4 textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
		draw(text, size, backgroundColor, textColor);
	}
	void draw(const char* text, ImVec4 backgroundColor, ImVec4 textColor){
		draw(text, ImVec2(0.0, 0.0), backgroundColor, textColor);
	}
	void draw(const char* text, ImVec2 size, ImVec4 backgroundColor, ImVec4 textColor){
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0.0, 0.0));
		ImGui::PushStyleColor(ImGuiCol_Button, backgroundColor);
		ImGui::PushStyleColor(ImGuiCol_Text, textColor);
		if(size.x == 0.0 && size.y == 0.0) ImGui::Button(text);
		else ImGui::Button(text, size);
		ImGui::PopStyleColor(2);
		ImGui::PopStyleVar();
		ImGui::PopItemFlag();
	}
}
