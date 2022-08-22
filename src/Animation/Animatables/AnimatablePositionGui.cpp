#include <pch.h>

#include "AnimatablePosition.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void AnimatablePosition::manualControlsVerticalGui(float sliderHeight, const char* customName){
		
	ImGui::BeginGroup();
	
	float sliderWidth = ImGui::GetTextLineHeight() * 1.75;
	float velocityDisplayWidth = ImGui::GetTextLineHeight() * 0.75;
	glm::vec2 verticalSliderSize(sliderWidth, sliderHeight);
	glm::vec2 velocityDisplaySize(velocityDisplayWidth, sliderHeight);
	float channelWidth = verticalSliderSize.x + velocityDisplaySize.x + ImGui::GetStyle().ItemSpacing.x;
	
	ImGui::PushID(getName());
	
	//--- Axis Name
	if(customName) {
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 nameFrameSize(channelWidth, ImGui::GetTextLineHeight());
		backgroundText(customName, nameFrameSize, Colors::darkGray);
		ImGui::PopFont();
	}
	
	//--- Velocity Slider & Feedback
	static const double min = -1.0;
	static const double max = 1.0;
	ImGui::VSliderScalar("##ManualVelocity", verticalSliderSize, ImGuiDataType_Double, &velocitySliderDisplayValue, &min, &max, "");
	if (ImGui::IsItemActive()) setManualVelocityTarget(velocitySliderDisplayValue);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setManualVelocityTarget(0.0);
		velocitySliderDisplayValue = 0.0;
	}
	ImGui::SameLine();
	verticalProgressBar(std::abs(getActualVelocityNormalized()), velocityDisplaySize);
	
	//--- Numerical Velocity & Position Feedback
	ImGui::PushFont(Fonts::sansRegular12);
	glm::vec2 feedbackFrameSize(channelWidth, ImGui::GetTextLineHeight() * 2.0);
	static char feedbackString[32];
	const char *positionUnitAbbreviated = getUnit()->abbreviated;
	sprintf(feedbackString, "%.3f%s\n%.2f%s/s",
			getActualPosition(),
			positionUnitAbbreviated,
			getActualVelocity(),
			positionUnitAbbreviated);
	backgroundText(feedbackString, feedbackFrameSize, Colors::darkGray);
	ImGui::PopFont();
	
	//--- Rapid Target Position Entry Box
	ImGui::SetNextItemWidth(channelWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", rapidTargetPositionDisplayValue, positionUnitAbbreviated);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1));
	ImGui::InputDouble("##TargetPosition", &rapidTargetPositionDisplayValue, 0.0, 0.0, targetPositionString);
	ImGui::PopStyleVar();
	if(ImGui::IsItemDeactivatedAfterEdit()) rapidTargetPositionDisplayValue = std::clamp(rapidTargetPositionDisplayValue, lowerPositionLimit, upperPositionLimit);
	if(isInRapid()){
		//display rapid progress if in rapid
		float rapidProgress = getRapidProgress();
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * rapidProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	
	//--- Stop and Rapid Buttons
	if(isControlledManuallyOrByAnimation()){
		if (ImGui::Button("Stop", ImVec2(channelWidth, ImGui::GetTextLineHeight() * 1.5))) {
			stopMovement();
		}
	}else{
		if (ImGui::Button("Rapid", ImVec2(channelWidth, ImGui::GetTextLineHeight() * 1.5))) {
			setManualPositionTargetWithVelocity(rapidTargetPositionDisplayValue, velocityLimit);
		}
	}
	
	ImGui::PopID();
	
	ImGui::EndGroup();
};

void AnimatablePosition::manualControlsHorizontalGui(float sliderWidth, const char* customName){
	ImGui::BeginGroup();

	float sliderHeight = ImGui::GetTextLineHeight() * 1.75;
	float velocityDisplayHeight = ImGui::GetTextLineHeight() * 0.75;
	glm::vec2 horizontalSliderSize(sliderWidth, sliderHeight);
	glm::vec2 velocityDisplaySize(sliderWidth, velocityDisplayHeight);
	float channelHeight = horizontalSliderSize.y + velocityDisplaySize.y + ImGui::GetStyle().ItemSpacing.y;

	ImGui::PushID(getName());

	//--- Velocity Slider & Feedback
	static const double min = -1.0;
	static const double max = 1.0;
	ImGui::BeginGroup();
	ImGui::SetNextItemWidth(sliderWidth);
	ImGui::SliderScalar("##ManualVelocity", ImGuiDataType_Double, &velocitySliderDisplayValue, &min, &max, "");
	if (ImGui::IsItemActive()) setManualVelocityTarget(velocitySliderDisplayValue);
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setManualVelocityTarget(0.0);
		velocitySliderDisplayValue = 0.0;
	}
	ImGui::ProgressBar(std::abs(getActualPositionNormalized()), velocityDisplaySize, "");
	ImGui::EndGroup();
	
	float controlsHeight = ImGui::GetItemRectSize().y;
	float textBoxWidth = ImGui::GetTextLineHeight() * 3.0;
	
	ImGui::SameLine();
	ImGui::BeginGroup();
	static char feedbackString[32];
	const char *positionUnitAbbreviated = getUnit()->abbreviated;
	sprintf(feedbackString, "%.3f%s\n%.2f%s/s",
			getActualPosition(),
			positionUnitAbbreviated,
			getActualVelocity(),
			positionUnitAbbreviated);
	ImGui::PushFont(Fonts::sansRegular12);
	if(customName){
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
		backgroundText(customName, glm::vec2(textBoxWidth, ImGui::GetTextLineHeight()), Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersTop);
		float nameHeight = ImGui::GetItemRectSize().y;
		backgroundText(feedbackString, glm::vec2(textBoxWidth, controlsHeight - nameHeight), Colors::veryDarkGray, Colors::white, ImDrawFlags_RoundCornersBottom);
		ImGui::PopStyleVar();
	}else{
		backgroundText(feedbackString, glm::vec2(textBoxWidth, controlsHeight), Colors::darkGray);
	}
	ImGui::PopFont();
	ImGui::EndGroup();
	
	
	ImGui::SameLine();
	ImGui::BeginGroup();
	
	
	 
	//--- Rapid Target Position Entry Box
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.f));
	ImGui::PushFont(Fonts::sansRegular12);
	float cursorHeight = ImGui::GetCursorPosY();
	
	ImGui::SetNextItemWidth(textBoxWidth);
	static char targetPositionString[32];
	sprintf(targetPositionString, "%.3f %s", rapidTargetPositionDisplayValue, positionUnitAbbreviated);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1));
	ImGui::InputDouble("##TargetPosition", &rapidTargetPositionDisplayValue, 0.0, 0.0, targetPositionString);
	ImGui::PopStyleVar();
	
	if(ImGui::IsItemDeactivatedAfterEdit()) rapidTargetPositionDisplayValue = std::clamp(rapidTargetPositionDisplayValue, lowerPositionLimit, upperPositionLimit);
	if(isInRapid()){
		//display rapid progress if in rapid
		float rapidProgress = getRapidProgress();
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * rapidProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	
	float textFieldHeight = ImGui::GetCursorPosY() - cursorHeight;
	float buttonHeight = controlsHeight - ImGui::GetStyle().ItemSpacing.y - textFieldHeight;
	
	//--- Stop and Rapid Buttons
	if(isControlledManuallyOrByAnimation()){
		if (ImGui::Button("Stop", ImVec2(textBoxWidth, buttonHeight))) stopMovement();
	}else{
		if (ImGui::Button("Rapid", ImVec2(textBoxWidth, buttonHeight))) setManualPositionTargetWithVelocity(rapidTargetPositionDisplayValue, velocityLimit);
	}
	
	ImGui::PopFont();
	ImGui::PopStyleVar();
	 
	ImGui::EndGroup();
	

	ImGui::PopID();

	ImGui::EndGroup();
}
