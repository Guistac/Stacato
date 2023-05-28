#include <pch.h>

#include "AnimatablePosition.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void AnimatablePosition::manualControlsVerticalGui(float sliderHeight, const char* customName, bool invert){
		
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
	if (ImGui::IsItemActive()) {
		float requestedVelocity = velocitySliderDisplayValue * velocityLimit;
		if(invert) requestedVelocity *= -1.0;
		setManualVelocityTarget(requestedVelocity);
	}
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setManualVelocityTarget(0.0);
		velocitySliderDisplayValue = 0.0;
	}
	ImGui::SameLine();
	verticalProgressBar(std::abs(getActualVelocityNormalized()), velocityDisplaySize);
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::Text("Velocity");
		ImGui::EndTooltip();
	}
	
	//--- Numerical Velocity & Position Feedback
	ImGui::PushFont(Fonts::sansRegular12);
	glm::vec2 feedbackFrameSize(channelWidth, ImGui::GetTextLineHeight());
	static char feedbackString[32];
	const char *positionUnitAbbreviated = getUnit()->abbreviated;
	if(state == Animatable::State::OFFLINE) sprintf(feedbackString, "-%s", positionUnitAbbreviated);
	else sprintf(feedbackString, "%.3f%s", getActualPosition(), positionUnitAbbreviated);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
	backgroundText(feedbackString, feedbackFrameSize, Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersTop);
	ImGui::PopStyleVar();
	double vel = getActualVelocity();
	if(std::abs(vel) < 0.01) vel = 0.0;
	if(state == Animatable::State::OFFLINE) sprintf(feedbackString, "-%s/s", positionUnitAbbreviated);
	else sprintf(feedbackString, "%.2f%s/s", vel, positionUnitAbbreviated);
	backgroundText(feedbackString, feedbackFrameSize, Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersBottom);
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
	
	const char* modestring;
	switch(controlMode){
		case VELOCITY_SETPOINT: 		modestring = "Velocity sp"; break;
		case POSITION_SETPOINT:			modestring = "Position sp"; break;
		case FORCED_POSITION_SETPOINT:	modestring = "Forced Position sp"; break;
		case FORCED_VELOCITY_SETPOINT:	modestring = "Force Velocity sp"; break;
	}
	
	/*
	ImGui::Text("anim: %i", hasAnimation());
	ImGui::Text("m: %s", modestring);
	ImGui::Text("p s: %.3f p: %.3f", positionSetpoint, motionProfile.getPosition());
	ImGui::Text("v s: %.3f p: %.3f", velocitySetpoint, motionProfile.getVelocity());
	 */
	
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
	if (ImGui::IsItemActive()) {
		float requesterVelocity = velocitySliderDisplayValue * velocityLimit;
		setManualVelocityTarget(requesterVelocity);
	}
	else if (ImGui::IsItemDeactivatedAfterEdit()) {
		setManualVelocityTarget(0.0);
		velocitySliderDisplayValue = 0.0;
	}
	ImGui::ProgressBar(std::abs(getActualVelocityNormalized()), velocityDisplaySize, "");
	if(ImGui::IsItemHovered()){
		ImGui::BeginTooltip();
		ImGui::Text("Velocity");
		ImGui::EndTooltip();
	}
	ImGui::EndGroup();
	
	
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(ImGui::GetStyle().ItemSpacing.y, 0.0));
	
	float controlsHeight = ImGui::GetTextLineHeight() * 2.0;
	float textBoxWidth = (sliderWidth - ImGui::GetStyle().ItemSpacing.x) / 2.0;
	
	ImGui::BeginGroup();
	static char positionString[32];
	static char velocityString[32];
	const char *positionUnitAbbreviated = getUnit()->abbreviated;
	if(state == Animatable::State::OFFLINE) snprintf(positionString, 32, "-%s", positionUnitAbbreviated);
	else snprintf(positionString, 32, "%.3f%s", getActualPosition(), positionUnitAbbreviated);
	if(state == Animatable::State::OFFLINE) snprintf(velocityString, 32, "-%s/s", positionUnitAbbreviated);
	else {
		double vel = getActualVelocity();
		if(std::abs(vel) < 0.01) vel = 0.0;
		snprintf(velocityString, 32, "%.2f%s/s", vel, positionUnitAbbreviated);
	}
	ImGui::PushFont(Fonts::sansRegular12);
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(0.0));
	glm::vec2 feedbackFrameSize(textBoxWidth, controlsHeight * .5f);
	backgroundText(positionString, feedbackFrameSize, Colors::veryDarkGray, Colors::white, ImDrawFlags_RoundCornersTop);
	backgroundText(velocityString, feedbackFrameSize, Colors::veryDarkGray, Colors::white, ImDrawFlags_RoundCornersBottom);
	ImGui::PopStyleVar();
	ImGui::PopFont();
	ImGui::EndGroup();
	
	ImGui::SameLine();
	ImGui::BeginGroup();
	
	ImGui::PopStyleVar();
	 
	//--- Rapid Target Position Entry Box
	ImGui::PushFont(Fonts::sansRegular12);
	float cursorHeight = ImGui::GetCursorPosY();
	ImGui::SetNextItemWidth(textBoxWidth);
	static char targetPositionString[32];
	snprintf(targetPositionString, 32, "%.3f %s", rapidTargetPositionDisplayValue, positionUnitAbbreviated);
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, glm::vec2(ImGui::GetStyle().FramePadding.x, ImGui::GetTextLineHeight() * 0.1));
	ImGui::InputDouble("##TargetPosition", &rapidTargetPositionDisplayValue, 0.0, 0.0, targetPositionString);
	ImGui::PopStyleVar();
	
	if(ImGui::IsItemDeactivatedAfterEdit()) {
		rapidTargetPositionDisplayValue = std::clamp(rapidTargetPositionDisplayValue, lowerPositionLimit, upperPositionLimit);
	}
	if(isInRapid()){
		//display rapid progress if in rapid
		float rapidProgress = getRapidProgress();
		glm::vec2 targetmin = ImGui::GetItemRectMin();
		glm::vec2 targetmax = ImGui::GetItemRectMax();
		glm::vec2 targetsize = ImGui::GetItemRectSize();
		glm::vec2 progressBarMax(targetmin.x + targetsize.x * rapidProgress, targetmax.y);
		ImGui::GetWindowDrawList()->AddRectFilled(targetmin, progressBarMax, ImColor(glm::vec4(1.0, 1.0, 1.0, 0.2)), 5.0);
	}
	
	float inputBoxHeight = ImGui::GetItemRectSize().y;
	float buttonHeight = controlsHeight - inputBoxHeight;
	ImGui::SetCursorPosY(cursorHeight + inputBoxHeight);
	
	//--- Stop and Rapid Buttons
	ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(0,0));
	if(isControlledManuallyOrByAnimation()){
		if (ImGui::Button("Stop", ImVec2(textBoxWidth, buttonHeight))) stopMovement();
	}else{
		if (ImGui::Button("Rapid", ImVec2(textBoxWidth, buttonHeight))) setManualPositionTargetWithVelocity(rapidTargetPositionDisplayValue, velocityLimit);
	}
	ImGui::PopStyleVar();
	
	ImGui::PopFont();
	 
	ImGui::EndGroup();
	

	ImGui::PopID();

	ImGui::EndGroup();
}
