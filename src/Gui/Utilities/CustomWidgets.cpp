#include <pch.h>

#include "CustomWidgets.h"

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
	bool ret;
	if (size == 0) size = ImGui::GetFrameHeight();
	
	ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	glm::vec2 center;
	float cross_extent;
    center = ImGui::GetItemRectMin();
    center += glm::vec2(ImGui::GetItemRectSize().x * 0.5);
    cross_extent = ImGui::GetTextLineHeight() * 0.5f * 0.7071f - 1.0f;
	
    ImGui::GetWindowDrawList()->AddLine(center + glm::vec2(+cross_extent, +cross_extent),
										center + glm::vec2(-cross_extent, -cross_extent),
										pictogramColor,
										ImGui::GetTextLineHeight() * 0.15);
    ImGui::GetWindowDrawList()->AddLine(center + glm::vec2(+cross_extent, -cross_extent),
										center + glm::vec2(-cross_extent, +cross_extent),
										pictogramColor,
										ImGui::GetTextLineHeight() * 0.15);
    return ret;
}

bool buttonPlay(const char* id, float size){
	bool ret;
	if (size == 0) size = ImGui::GetFrameHeight();
	
	ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	float padding = size * 0.25;
	ImVec2 points[3] = {
		ImVec2(min.x + padding, min.y + padding),
		ImVec2(min.x + padding, min.y + size - padding),
		ImVec2(min.x + size - padding, min.y + size / 2.0)
	};
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
	return ret;
}

bool buttonPause(const char* id, float size){
	bool ret;
	if (size == 0) size = ImGui::GetFrameHeight();
	
	ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	float padding = size * 0.25;
	float barWidth = size * 0.15;
	float spaceWidth = size * 0.15;
	float totalWidth = spaceWidth + barWidth * 2.0;
	float x1 = min.x + (size - totalWidth) / 2.0;
	float x2 = x1 + barWidth + spaceWidth;
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x1, min.y + padding), ImVec2(x1 + barWidth, max.y - padding), pictogramColor);
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(x2, min.y + padding), ImVec2(x2 + barWidth, max.y - padding), pictogramColor);
	return ret;
}

bool buttonStop(const char* id, float size){
	if (size == 0) size = ImGui::GetFrameHeight();
	
	bool output = ImGui::InvisibleButton(id, ImVec2(size, size));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	float padding = size * 0.25;
	float barWidth = size * 0.15;
	float spaceWidth = size * 0.15;
	float totalWidth = spaceWidth + barWidth * 2.0;
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + padding), ImVec2(max.x - padding, max.y - padding), pictogramColor);
	return output;
}

bool buttonSTOP(const char* id, float size){
	bool ret;
	if (size == 0) size = ImGui::GetFrameHeight();
	
	ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	float padding = size * 0.25;
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + padding), ImVec2(max.x - padding, max.y - padding), pictogramColor);
		
	ImVec2 center((max.x + min.x) / 2.0, (max.y + min.y) / 2.0);
	center.x -= 0.5;
	center.y -= 0.5;
	float cross_extent = size * 0.175;
	float crossWidth = size * 0.1;
	ImColor crossColor = ImColor(0.3f, 0.3f, 0.3f, 1.0f);
	
	ImGui::GetWindowDrawList()->AddLine(ImVec2(center.x - cross_extent, center.y - cross_extent),
										ImVec2(center.x + cross_extent, center.y + cross_extent),
										crossColor,
										ImGui::GetTextLineHeight() * 0.1);
	ImGui::GetWindowDrawList()->AddLine(ImVec2(center.x + cross_extent, center.y - cross_extent),
										ImVec2(center.x - cross_extent, center.y + cross_extent),
										crossColor,
										ImGui::GetTextLineHeight() * 0.1);
	
	return ret;
}

bool buttonArrowLeft(const char* id, float size){
	bool ret;
	if (size == 0) ret = ImGui::Button(id, glm::vec2(ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0));
	else ret = ImGui::Button(id, glm::vec2(size));
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImVec2 points[3] = {
		ImVec2(min.x + padding, 				min.y + (size / 2.0)),
		ImVec2(min.x + padding + triangleSize, 	min.y + (size / 2.0) - (triangleSize / 2.0)),
		ImVec2(min.x + padding + triangleSize, 	min.y + (size / 2.0) + (triangleSize / 2.0))
	};
	
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding + triangleSize - 1, min.y + (size - barHeight) / 2.0),
											  ImVec2(max.x - padding, max.y - (size - barHeight) / 2.0),
											  ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	return ret;
}

bool buttonArrowRight(const char* id, float size){
	bool ret;
	if (size == 0) ret = ImGui::Button(id, glm::vec2(ImGui::GetTextLineHeight() + ImGui::GetStyle().FramePadding.y * 2.0));
	else ret = ImGui::Button(id, glm::vec2(size));
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImVec2 points[3] = {
		ImVec2(max.x - padding, 				max.y - (size / 2.0)),
		ImVec2(max.x - padding - triangleSize, 	max.y - (size / 2.0) - (triangleSize / 2.0)),
		ImVec2(max.x - padding - triangleSize, 	max.y - (size / 2.0) + (triangleSize / 2.0))
	};
	
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + (size - barHeight) / 2.0),
											  ImVec2(max.x - padding - triangleSize + 1, max.y - (size - barHeight) / 2.0),
											  ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	return ret;
}






bool buttonArrowRightStop(const char* id, float size){
	if (size == 0) size = ImGui::GetFrameHeight();
	
	bool ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	float vertBarWidth = size * 0.05;
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(max.x - padding - vertBarWidth, 	min.y + padding),
											  ImVec2(max.x - padding, 					max.y - padding),
											  pictogramColor);
	
	ImVec2 points[3] = {
		ImVec2(max.x - padding - vertBarWidth, 					max.y - (size / 2.0)),
		ImVec2(max.x - padding - triangleSize - vertBarWidth, 	max.y - (size / 2.0) - (triangleSize / 2.0)),
		ImVec2(max.x - padding - triangleSize - vertBarWidth, 	max.y - (size / 2.0) + (triangleSize / 2.0))
	};
	
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, 									min.y + (size - barHeight) / 2.0),
											  ImVec2(max.x - padding - triangleSize - vertBarWidth + 1, max.y - (size - barHeight) / 2.0),
											  pictogramColor);
	return ret;
}

bool buttonArrowLeftStop(const char* id, float size){
	if (size == 0) size = ImGui::GetFrameHeight();
	
	bool ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	float vertBarWidth = size * 0.05;
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + padding),
											  ImVec2(min.x + padding + vertBarWidth, max.y - padding),
											  pictogramColor);
	
	ImVec2 points[3] = {
		ImVec2(min.x + padding + vertBarWidth, 					min.y + (size / 2.0)),
		ImVec2(min.x + padding + vertBarWidth + triangleSize, 	min.y + (size / 2.0) - (triangleSize / 2.0)),
		ImVec2(min.x + padding + vertBarWidth + triangleSize, 	min.y + (size / 2.0) + (triangleSize / 2.0))
	};
	
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + vertBarWidth + padding + triangleSize - 1, 	min.y + (size - barHeight) / 2.0),
											  ImVec2(max.x - padding,										max.y - (size - barHeight) / 2.0),
											  pictogramColor);
	return ret;
}

bool buttonArrowDownStop(const char* id, float size){
	if (size == 0) size = ImGui::GetFrameHeight();
	
	bool ret = ImGui::InvisibleButton(id, ImVec2(size, size));
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	float barWidth = size * 0.05;
	
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersAll);
	
	ImColor pictogramColor;
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding,	max.y - padding - barWidth),
											  ImVec2(max.x - padding, 	max.y - padding),
											  pictogramColor);
	
	ImVec2 points[3] = {
		ImVec2(min.x + (size / 2.0), 							max.y - padding - barWidth 	),
		ImVec2(min.x + (size / 2.0) - (triangleSize / 2.0),		max.y - padding - barWidth - triangleSize),
		ImVec2(min.x + (size / 2.0) + (triangleSize / 2.0),		max.y - padding - barWidth - triangleSize)
	};
	
	ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + (size - barHeight) / 2.0,	min.y + padding	),
											  ImVec2(max.x - (size - barHeight) / 2.0,	max.y - barWidth - padding - triangleSize + 1),
											  pictogramColor);
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


namespace UpDownButtons{
	Interaction draw(const char* ID, float buttonHeight, bool disableUp, bool disableDown){
		ImGui::PushID(ID);
		ImGui::BeginGroup();
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0.0,0.0));
		
		if(buttonHeight <= 0.0) buttonHeight = ImGui::GetFrameHeight();
		
		ImVec2 min, max;
		ImU32 buttonColor;
		ImColor pictogramColor;
		float triangleSpacing = 0.3;
		float rounding = ImGui::GetStyle().FrameRounding;
		ImDrawList* drawList = ImGui::GetWindowDrawList();
		ImVec2 windowPos = ImGui::GetWindowPos();
		float separatorSize = 1.0;
		
		Interaction output = Interaction::NONE;
		
		ImGui::BeginDisabled(disableUp);
		if(ImGui::InvisibleButton("##Up", ImVec2(buttonHeight, buttonHeight))) output = Interaction::UP;
		min = ImGui::GetItemRectMin();
		max = ImGui::GetItemRectMax();
		buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		
		if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
		else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		
		drawList->AddRectFilled(min, max, buttonColor, rounding, ImDrawFlags_RoundCornersTop);
		drawList->AddTriangleFilled(
									ImVec2(min.x + buttonHeight * 0.5f, min.y + buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight - buttonHeight * triangleSpacing, min.y + buttonHeight - buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight * triangleSpacing, min.y + buttonHeight - buttonHeight * triangleSpacing),
									pictogramColor);
		ImGui::EndDisabled();
		
		ImGui::SetCursorPosX(min.x - windowPos.x);
		ImGui::SetCursorPosY(max.y - windowPos.y + separatorSize);
		
		ImGui::BeginDisabled(disableDown);
		if(ImGui::InvisibleButton("##Down", ImVec2(buttonHeight, buttonHeight))) output = Interaction::DOWN;
		min = ImGui::GetItemRectMin();
		max = ImGui::GetItemRectMax();
		max.y -= separatorSize;
		buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
		
		if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
		else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		
		drawList->AddRectFilled(min, max, buttonColor, rounding, ImDrawFlags_RoundCornersBottom);
		drawList->AddTriangleFilled(
									ImVec2(min.x + buttonHeight * 0.5f, min.y + buttonHeight - buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight * triangleSpacing, min.y + buttonHeight * triangleSpacing),
									ImVec2(min.x + buttonHeight - buttonHeight * triangleSpacing, min.y + buttonHeight * triangleSpacing),
									pictogramColor);
		ImGui::EndDisabled();
		
		ImGui::PopStyleVar();
		ImGui::EndGroup();
		ImGui::PopID();
		
		return output;
	}
};


bool nextButton(const char* ID, float size, bool withStop, ImDrawFlags drawFlags){
	if(size <= 0.0) size = ImGui::GetFrameHeight();
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	float vertBarWidth = size * 0.05;
	
	ImVec2 min;
	ImVec2 max;
	
	ImColor buttonColor;
	ImColor pictogramColor;
	float rounding = ImGui::GetStyle().FrameRounding;
	
	bool output = ImGui::InvisibleButton(ID, ImVec2(size, size));
	min = ImGui::GetItemRectMin();
	max = ImGui::GetItemRectMax();
	
	buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, rounding, drawFlags);
	
	if(withStop){
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(max.x - padding - vertBarWidth, 	min.y + padding),
												  ImVec2(max.x - padding, 					max.y - padding),
												  pictogramColor);
		
		ImVec2 points[3] = {
			ImVec2(max.x - padding - vertBarWidth, 					max.y - (size / 2.0)),
			ImVec2(max.x - padding - triangleSize - vertBarWidth, 	max.y - (size / 2.0) - (triangleSize / 2.0)),
			ImVec2(max.x - padding - triangleSize - vertBarWidth, 	max.y - (size / 2.0) + (triangleSize / 2.0))
		};
		
		ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
		
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, 									min.y + (size - barHeight) / 2.0),
												  ImVec2(max.x - padding - triangleSize - vertBarWidth + 1, max.y - (size - barHeight) / 2.0),
												  pictogramColor);
	}else{
		ImVec2 pointsNext[3] = {
			ImVec2(max.x - padding, 				max.y - (size / 2.0)),
			ImVec2(max.x - padding - triangleSize, 	max.y - (size / 2.0) - (triangleSize / 2.0)),
			ImVec2(max.x - padding - triangleSize, 	max.y - (size / 2.0) + (triangleSize / 2.0))
		};
			
		ImGui::GetWindowDrawList()->AddConvexPolyFilled(pointsNext, 3, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + (size - barHeight) / 2.0),
												  ImVec2(max.x - padding - triangleSize + 1, max.y - (size - barHeight) / 2.0),
												  ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
		
	return output;
}

bool previousButton(const char* ID, float size, bool withStop, ImDrawFlags drawFlags){
	if(size <= 0.0) size = ImGui::GetFrameHeight();
	
	float padding = size * 0.15;
	float triangleSize = size * 0.4;
	float barHeight = size * 0.15;
	float vertBarWidth = size * 0.05;
	
	ImVec2 min;
	ImVec2 max;
	
	ImColor buttonColor;
	ImColor pictogramColor;
	float rounding = ImGui::GetStyle().FrameRounding;
	
	
	bool output = ImGui::InvisibleButton(ID, ImVec2(size, size));
	min = ImGui::GetItemRectMin();
	max = ImGui::GetItemRectMax();
	ImVec2 windowPos = ImGui::GetWindowPos();
	
	buttonColor = ImGui::GetColorU32( ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	if(ImGui::GetItemFlags() && ImGuiItemFlags_Disabled) pictogramColor = ImColor(0.5f, 0.5f, 0.5f, 1.0f);
	else pictogramColor = ImGui::GetColorU32(ImGuiCol_Text);
		
	ImGui::GetWindowDrawList()->AddRectFilled(min, max, buttonColor, rounding, drawFlags);
	
	if(withStop){
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding, min.y + padding),
												  ImVec2(min.x + padding + vertBarWidth, max.y - padding),
												  pictogramColor);
		
		ImVec2 points[3] = {
			ImVec2(min.x + padding + vertBarWidth, 					min.y + (size / 2.0)),
			ImVec2(min.x + padding + vertBarWidth + triangleSize, 	min.y + (size / 2.0) - (triangleSize / 2.0)),
			ImVec2(min.x + padding + vertBarWidth + triangleSize, 	min.y + (size / 2.0) + (triangleSize / 2.0))
		};
		
		ImGui::GetWindowDrawList()->AddConvexPolyFilled(points, 3, pictogramColor);
		
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + vertBarWidth + padding + triangleSize - 1, 	min.y + (size - barHeight) / 2.0),
												  ImVec2(max.x - padding,										max.y - (size - barHeight) / 2.0),
												  pictogramColor);
	}else{
		ImVec2 pointsPrevious[3] = {
			ImVec2(min.x + padding, 				min.y + (size / 2.0)),
			ImVec2(min.x + padding + triangleSize, 	min.y + (size / 2.0) - (triangleSize / 2.0)),
			ImVec2(min.x + padding + triangleSize, 	min.y + (size / 2.0) + (triangleSize / 2.0))
		};
		
		ImGui::GetWindowDrawList()->AddConvexPolyFilled(pointsPrevious, 3, ImColor(1.0f, 1.0f, 1.0f, 1.0f));
		
		ImGui::GetWindowDrawList()->AddRectFilled(ImVec2(min.x + padding + triangleSize - 1, min.y + (size - barHeight) / 2.0),
												  ImVec2(max.x - padding, max.y - (size - barHeight) / 2.0),
												  ImColor(1.0f, 1.0f, 1.0f, 1.0f));
	}
	
	return output;
}









void backgroundText(const char* text){
	backgroundText(text, ImVec2(0.0, 0.0));
}
void backgroundText(const char* text, ImVec2 size){
	glm::vec4 buttonColor = ImGui::GetStyle().Colors[ImGuiCol_Button];
	glm::vec4 textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
	backgroundText(text, size, buttonColor, textColor);
}
void backgroundText(const char* text, ImVec4 backgroundColor){
	backgroundText(text, ImVec2(0.0, 0.0), backgroundColor);
}
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor){
	glm::vec4 textColor = ImGui::GetStyle().Colors[ImGuiCol_Text];
	backgroundText(text, size, backgroundColor, textColor);
}
void backgroundText(const char* text, ImVec4 backgroundColor, ImVec4 textColor){
	backgroundText(text, ImVec2(0.0, 0.0), backgroundColor, textColor);
}
void backgroundText(const char* text, ImVec2 size, ImVec4 backgroundColor, ImVec4 textColor, ImDrawFlags drawFlags){
	ImVec2 textSize = ImGui::CalcTextSize(text);
	if(size.x <= 0.0 && size.y <= 0.0) {
		ImVec2 padding = ImGui::GetStyle().FramePadding;
		size = textSize;
		size.x += padding.x * 2.0;
		size.y += padding.y * 2.0;
	}else if(size.x <= 0.0){
		ImVec2 padding = ImGui::GetStyle().FramePadding;
		size.x = textSize.x + padding.x * 2.0;
	}
		
	ImGui::InvisibleButton(text, size);
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	drawing->AddRectFilled(min,
						   max,
						   ImColor(backgroundColor),
						   ImGui::GetStyle().FrameRounding,
						   drawFlags);
	
	ImVec2 position(min.x + (size.x - textSize.x) / 2.0, min.y + (size.y - textSize.y) / 2.0);
	ImGui::PushClipRect(min, max, true);
	drawing->AddText(position, ImColor(textColor), text);
	ImGui::PopClipRect();
	
}

bool textInputCustom(const char* ID, char* buffer, size_t bufferSize, ImVec2 size, ImGuiInputTextFlags flags){
	float paddingY = (size.y - ImGui::GetTextLineHeight()) / 2.0;
	float paddingX = ImGui::GetStyle().FramePadding.x;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(paddingX, paddingY));
	ImGui::SetNextItemWidth(size.x);
	ImGui::InputText(ID, buffer, bufferSize, flags);
	bool ret = ImGui::IsItemDeactivatedAfterEdit();
	ImGui::PopStyleVar();
	return ret;
}

#include "Animation/Playback/TimeStringConversion.h"

bool timeEntryWidgetSeconds(const char* ID, float height, double& time_seconds, const char* message){
	static char textBuffer[64];
	if(message) strcpy(textBuffer, message);
	else strcpy(textBuffer, TimeStringConversion::secondsToTimecodeString(time_seconds).c_str());
	
	ImVec2 size(ImGui::CalcTextSize("+00:00:00.0").x + ImGui::GetStyle().FramePadding.x * 2.0, height);
	if(textInputCustom(ID, textBuffer, 64, size, ImGuiInputTextFlags_AutoSelectAll)){
		time_seconds = TimeStringConversion::timecodeStringToSeconds(textBuffer);
		return true;
	}
	return false;
}

bool timeEntryWidgetMicroseconds(const char* ID, float height, long long int time_microseconds){
	static char textBuffer[64];
	strcpy(textBuffer, TimeStringConversion::microsecondsToTimecodeString(time_microseconds).c_str());
	bool ret = false;
	
	float paddingY = (height - ImGui::GetTextLineHeight()) / 2.0;
	float paddingX = ImGui::GetStyle().FramePadding.x;
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(paddingX, paddingY));
	ImGui::SetNextItemWidth(ImGui::CalcTextSize("+00:00:00.0").x + ImGui::GetStyle().FramePadding.x * 2.0);
	ImGui::InputText(ID, textBuffer, 64, ImGuiInputTextFlags_AutoSelectAll);
	
	if(ImGui::IsItemDeactivatedAfterEdit()){
		time_microseconds = TimeStringConversion::timecodeStringToMicroseconds(textBuffer);
		ret = true;
	}
	ImGui::PopStyleVar();
	return ret;
}

void scrollingTextBase(const char* ID, const char* text, ImVec2 size, bool lock, float pauseDuration, float velocity){
	
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	
	ImGui::PushID("ScrollingText");
	ImGuiID textLengthID = ImGui::GetID(ID);
	ImGuiID startTimeID = textLengthID + 1;
	ImGui::PopID();
	ImGuiStorage* storage = ImGui::GetStateStorage();
	
	int previousTextLength = storage->GetInt(textLengthID, -1);
	int currentTextLength = strlen(text);
	bool restart = false;
	if(previousTextLength != currentTextLength){
		storage->SetInt(textLengthID, currentTextLength);
		restart = true;
	}
	
	float startTime;
	if(lock || restart){
		startTime = Timing::getProgramTime_seconds();
		storage->SetFloat(startTimeID, startTime);
	}else{
		startTime = storage->GetFloat(startTimeID, -1.0);
	}
	
	ImVec2 textSize = ImGui::CalcTextSize(text);
	
	ImGui::PushClipRect(min, max, true);
	
	if(textSize.x < size.x){
		ImVec2 textPosition(min.x + (size.x - textSize.x) / 2.0, min.y + (size.y - textSize.y) / 2.0);
		ImGui::GetWindowDrawList()->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), text);
	}else{
		
		float spacing = ImGui::GetTextLineHeight();
		float scrollDistance = textSize.x + spacing;
		float scrollVelocity = ImGui::GetTextLineHeight() * velocity;
		float scrollDuration = (scrollDistance / scrollVelocity);
		float totalDuration = scrollDuration + pauseDuration;
		float time = Timing::getProgramTime_seconds() - startTime - pauseDuration;
		float scrollNormalized = fmod(time, totalDuration) / scrollDuration;
		if(scrollNormalized < 0.0 || scrollNormalized > 1.0) scrollNormalized = 0.0;
		float scrollOffset = scrollNormalized * scrollDistance;
		
		auto textColor = ImGui::GetColorU32(ImGuiCol_Text);
		glm::vec2 textPosition(min.x + ImGui::GetStyle().FramePadding.x, min.y + (size.y - textSize.y) / 2.0);
		textPosition.x -= scrollOffset;
		textPosition.x = std::round(textPosition.x);
		ImGui::GetWindowDrawList()->AddText(textPosition, textColor, text);
		glm::vec2 overlapTextPosition(textPosition.x + scrollDistance, textPosition.y);
		ImGui::GetWindowDrawList()->AddText(overlapTextPosition, textColor, text);
	}
	
	ImGui::PopClipRect();
	
	
}

void scrollingTextWithBackground(const char* ID, const char* text, ImVec2 size, ImVec4 backgroundColor, bool lock, float pauseDuration, float velocity){
	ImGui::InvisibleButton(text, size);
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	ImGui::GetWindowDrawList()->AddRectFilled(min, max,
											  ImColor(backgroundColor),
											  ImGui::GetStyle().FrameRounding,
											  ImDrawFlags_RoundCornersAll);
	scrollingTextBase(ID, text, size, lock, pauseDuration, velocity);
}

void scrollingText(const char* ID, const char* text, float width, bool lock, float pauseDuration, float velocity){
	ImVec2 size(width, ImGui::GetTextLineHeight());
	ImGui::InvisibleButton(text, size);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
	scrollingTextBase(ID, text, size, lock, pauseDuration, velocity);
	ImGui::PopStyleVar();
}


float verticalSeparator(float width, bool drawLine, bool allowMoving){
	float height = ImGui::GetContentRegionAvail().y;
	if(height <= 0.0) return;
	float output = 0.0;
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
	ImGui::SameLine();
	ImGui::InvisibleButton("##Divider", ImVec2(width, height));
	if(allowMoving && ImGui::IsItemActive() && ImGui::IsMouseDragging(ImGuiMouseButton_Left)){
		ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
		output = ImGui::GetIO().MouseDelta.x;
	}
	if(ImGui::IsItemActive() || ImGui::IsItemHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
	if(drawLine){
		ImVec2 buttonMin = ImGui::GetItemRectMin();
		ImVec2 buttonMax = ImGui::GetItemRectMax();
		ImVec2 buttonSize = ImGui::GetItemRectSize();
		float middleX = buttonMin.x + buttonSize.x / 2.0;
		ImGui::GetWindowDrawList()->AddLine(ImVec2(middleX, buttonMin.y), ImVec2(middleX, buttonMax.y), ImGui::GetColorU32(ImGuiCol_Separator));
	}
	ImGui::SameLine();
	ImGui::PopStyleVar();
	return output;
}

float verticalSeparatorNonResizeable(float width, float height){
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0,0));
	ImGui::SameLine();
	ImGui::InvisibleButton("##Divider", ImVec2(width, height));
	ImVec2 buttonMin = ImGui::GetItemRectMin();
	ImVec2 buttonMax = ImGui::GetItemRectMax();
	ImVec2 buttonSize = ImGui::GetItemRectSize();
	float middleX = buttonMin.x + buttonSize.x / 2.0;
	ImGui::GetWindowDrawList()->AddLine(ImVec2(middleX, buttonMin.y), ImVec2(middleX, buttonMax.y), ImGui::GetColorU32(ImGuiCol_Separator));
	ImGui::SameLine();
	ImGui::PopStyleVar();
}

void centeredText(const char* string, ImVec2 size){
	ImGui::Dummy(size);
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 textSize = ImGui::CalcTextSize(string);
	ImVec2 position(min.x + (size.x - textSize.x) / 2.0, min.y + (size.y - textSize.y) / 2.0);
	ImGui::PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
	ImGui::GetWindowDrawList()->AddText(position, ImGui::GetColorU32(ImGuiCol_Text), string);
	ImGui::PopClipRect();
}

bool customRoundedButton(const char* string, ImVec2 size, float rounding, ImDrawFlags whichCornersRounded){
	
	bool ret = ImGui::InvisibleButton(string, size);
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	
	ImU32 color = ImGui::GetColorU32(ImGui::IsItemActive() ? ImGuiCol_ButtonActive : ImGui::IsItemHovered() ? ImGuiCol_ButtonHovered : ImGuiCol_Button);
	
	ImGui::GetWindowDrawList()->AddRectFilled(ImGui::GetItemRectMin(),
											  ImGui::GetItemRectMax(),
											  color,
											  rounding,
											  whichCornersRounded);
	
	ImVec2 textSize = ImGui::CalcTextSize(string);
	ImVec2 position(min.x + (size.x - textSize.x) / 2.0, min.y + (size.y - textSize.y) / 2.0);
	ImGui::PushClipRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax(), true);
	ImGui::GetWindowDrawList()->AddText(position, ImGui::GetColorU32(ImGuiCol_Text), string);
	ImGui::PopClipRect();
	
	return ret;
}



ImVec2 frameStart;
ImVec2 framePadding;
ImVec4 frameColor;
float frameOutlineWidth;
ImVec4 frameOutlineColor;
ImDrawList* frameDrawing;
ImDrawListSplitter frameDrawingLayers;
ImDrawFlags frameDrawFlags;


void startFrame(ImVec4 backgroundColor, float outlineWidth, ImVec4 outlineColor, ImVec2 outerSize, ImDrawFlags drawFlags){
	frameDrawing = ImGui::GetWindowDrawList();
	frameDrawingLayers.Split(frameDrawing, 2);
	frameDrawingLayers.SetCurrentChannel(frameDrawing, 1);
	frameStart = ImGui::GetCursorPos();
	framePadding = ImGui::GetStyle().FramePadding;
	frameColor = backgroundColor;
	frameOutlineWidth = outlineWidth;
	frameOutlineColor = outlineColor;
	frameDrawFlags = drawFlags;
	ImGui::BeginGroup();
	ImGui::SetCursorPos(ImVec2(frameStart.x + framePadding.x, frameStart.y + framePadding.y));
	ImGui::BeginGroup();
	ImVec2 contentSize(outerSize.x - framePadding.x * 2.0, outerSize.y - framePadding.y * 2.0);
	if(contentSize.x > 0.0 || contentSize.x > 0.0){
		ImVec2 cursor = ImGui::GetCursorPos();
		ImGui::Dummy(contentSize);
		ImGui::SetCursorPos(cursor);
	}
}

void endFrame(){
	ImGui::EndGroup();
	ImVec2 contentSize = ImGui::GetItemRectSize();
	ImGui::SetCursorPos(ImVec2(frameStart.x + framePadding.x * 2.0 + contentSize.x, frameStart.y + framePadding.y * 2.0 + contentSize.y));
	ImGui::EndGroup();
	ImVec2 min = ImGui::GetItemRectMin();
	ImVec2 max = ImGui::GetItemRectMax();
	frameDrawingLayers.SetCurrentChannel(frameDrawing, 0);
	frameDrawing->AddRectFilled(min, max, ImColor(frameColor), ImGui::GetStyle().FrameRounding, frameDrawFlags);
	if(frameOutlineWidth > 0.0) frameDrawing->AddRect(min, max, ImColor(frameOutlineColor), ImGui::GetStyle().FrameRounding, frameDrawFlags, frameOutlineWidth);
	frameDrawingLayers.Merge(frameDrawing);
}




void textAligned(const char* txt, ImVec2 position, TextAlignement alignement, ImVec2 minBounding, ImVec2 maxBounding){
	if(txt == nullptr) return;
	ImVec2 textSize = ImGui::CalcTextSize(txt);
	ImVec2 drawPosition;
	switch(alignement){
		case TextAlignement::LEFT_TOP:
		case TextAlignement::LEFT_MIDDLE:
		case TextAlignement::LEFT_BOTTOM:
			drawPosition.x = position.x;
			break;
		case TextAlignement::MIDDLE_TOP:
		case TextAlignement::MIDDLE_MIDDLE:
		case TextAlignement::MIDDLE_BOTTOM:
			drawPosition.x = position.x - textSize.x * .5f;
			break;
		case TextAlignement::RIGHT_TOP:
		case TextAlignement::RIGHT_MIDDLE:
		case TextAlignement::RIGHT_BOTTOM:
			drawPosition.x = position.x - textSize.x;
			break;
	}
	switch(alignement){
		case TextAlignement::LEFT_TOP:
		case TextAlignement::MIDDLE_TOP:
		case TextAlignement::RIGHT_TOP:
			drawPosition.y = position.y;
			break;
		case TextAlignement::LEFT_MIDDLE:
		case TextAlignement::MIDDLE_MIDDLE:
		case TextAlignement::RIGHT_MIDDLE:
			drawPosition.y = position.y - textSize.y * .5f;
			break;
		case TextAlignement::LEFT_BOTTOM:
		case TextAlignement::MIDDLE_BOTTOM:
		case TextAlignement::RIGHT_BOTTOM:
			drawPosition.y = position.y - textSize.y;
			break;
	}
	
	drawPosition.x = std::max(drawPosition.x, minBounding.x);
	drawPosition.y = std::max(drawPosition.y, minBounding.x);
	drawPosition.x = std::min(drawPosition.x, maxBounding.x - textSize.x);
	drawPosition.y = std::min(drawPosition.y, maxBounding.y - textSize.y);
	ImGui::GetWindowDrawList()->AddText(drawPosition, ImGui::GetColorU32(ImGuiCol_Text), txt);
}


void textAlignedBackground(const char* txt,
						   ImVec2 position,
						   TextAlignement alignement,
						   ImVec4 backgroundColor,
						   ImVec2 padding,
						   float rounding,
						   ImDrawFlags drawFlags,
						   bool restrictToBounds,
						   ImVec2 minBounding,
						   ImVec2 maxBounding){
	
	if(txt == nullptr) return;
	ImVec2 textSize = ImGui::CalcTextSize(txt);
	ImVec2 boxSize = ImVec2(textSize.x + padding.x * 2.f, textSize.y + padding.y * 2.f);
	ImVec2 boxPositionMin;
	
	switch(alignement){
		case TextAlignement::LEFT_TOP:
		case TextAlignement::LEFT_MIDDLE:
		case TextAlignement::LEFT_BOTTOM:
			boxPositionMin.x = position.x;
			break;
		case TextAlignement::MIDDLE_TOP:
		case TextAlignement::MIDDLE_MIDDLE:
		case TextAlignement::MIDDLE_BOTTOM:
			boxPositionMin.x = position.x - boxSize.x * .5f;
			break;
		case TextAlignement::RIGHT_TOP:
		case TextAlignement::RIGHT_MIDDLE:
		case TextAlignement::RIGHT_BOTTOM:
			boxPositionMin.x = position.x - boxSize.x;
			break;
	}
	switch(alignement){
		case TextAlignement::LEFT_TOP:
		case TextAlignement::MIDDLE_TOP:
		case TextAlignement::RIGHT_TOP:
			boxPositionMin.y = position.y;
			break;
		case TextAlignement::LEFT_MIDDLE:
		case TextAlignement::MIDDLE_MIDDLE:
		case TextAlignement::RIGHT_MIDDLE:
			boxPositionMin.y = position.y - boxSize.y * .5f;
			break;
		case TextAlignement::LEFT_BOTTOM:
		case TextAlignement::MIDDLE_BOTTOM:
		case TextAlignement::RIGHT_BOTTOM:
			boxPositionMin.y = position.y - boxSize.y;
			break;
	}
	
	if(restrictToBounds){
		boxPositionMin.x = std::max(boxPositionMin.x, minBounding.x);
		boxPositionMin.y = std::max(boxPositionMin.y, minBounding.y);
		boxPositionMin.x = std::min(boxPositionMin.x, maxBounding.x - boxSize.x);
		boxPositionMin.y = std::min(boxPositionMin.y, maxBounding.y - boxSize.y);
	}
	 
	ImVec2 textPosition = ImVec2(boxPositionMin.x + padding.x, boxPositionMin.y + padding.y);
	ImVec2 boxPositionMax = ImVec2(boxPositionMin.x + boxSize.x, boxPositionMin.y + boxSize.y);
	
	ImDrawList* drawing = ImGui::GetWindowDrawList();
	drawing->AddRectFilled(boxPositionMin, boxPositionMax, ImColor(backgroundColor), rounding, drawFlags);
	drawing->AddText(textPosition, ImGui::GetColorU32(ImGuiCol_Text), txt);
}


bool customButton(const char* txt, ImVec2 size, ImVec4 color, float rounding, ImDrawFlags drawFlags){
    bool pressed = ImGui::InvisibleButton(txt, size);

    ImColor buttonColor;
	if(ImGui::IsItemActive()) buttonColor = ImGui::GetColorU32(ImGuiCol_ButtonActive);
	else if(ImGui::IsItemHovered()) buttonColor = ImGui::GetColorU32(ImGuiCol_ButtonHovered);
	else buttonColor = ImColor(color);
    
    ImVec2 min = ImGui::GetItemRectMin();
    ImVec2 max = ImGui::GetItemRectMax();
    ImVec2 textSize = ImGui::CalcTextSize(txt);
    ImVec2 textPos = ImVec2(min.x + (size.x - textSize.x) * .5f,
                            min.y + (size.y - textSize.y) * .5f);
    
    ImDrawList* drawing = ImGui::GetWindowDrawList();
    drawing->AddRectFilled(min, max, buttonColor, rounding, drawFlags);
    ImGui::PushClipRect(min, max, true);
    drawing->AddText(textPos, ImGui::GetColorU32(ImGuiCol_Text), txt);
    ImGui::PopClipRect();
    
    return pressed;
}
