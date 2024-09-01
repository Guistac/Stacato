#include <pch.h>

#include "AnimatableVelocity.h"

#include <imgui.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"
#include "Gui/Utilities/CustomWidgets.h"

void AnimatableVelocity::manualControlsVerticalGui(float sliderHeight, const char* customName, bool invert){
		
	ImGui::InvisibleButton("Spacer", ImVec2(ImGui::GetTextLineHeight()*1.2, 1.0));
	ImGui::SameLine();
	
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
	double vel = getActualVelocity();
	if(std::abs(vel) < 0.01) vel = 0.0;
	if(state == Animatable::State::OFFLINE) sprintf(feedbackString, "-%s/s", positionUnitAbbreviated);
	else sprintf(feedbackString, "%.2f%s/s", vel, positionUnitAbbreviated);
	backgroundText(feedbackString, feedbackFrameSize, Colors::darkGray, Colors::white, ImDrawFlags_RoundCornersBottom);
	ImGui::PopFont();
	
	/*
	const char* modestring;
	switch(controlMode){
		case VELOCITY_SETPOINT: 		modestring = "Velocity sp"; break;
		case POSITION_SETPOINT:			modestring = "Position sp"; break;
		case FORCED_POSITION_SETPOINT:	modestring = "Forced Position sp"; break;
		case FORCED_VELOCITY_SETPOINT:	modestring = "Force Velocity sp"; break;
	}
	
	ImGui::Text("anim: %i", hasAnimation());
	ImGui::Text("m: %s", modestring);
	ImGui::Text("p s: %.3f p: %.3f", positionSetpoint, motionProfile.getPosition());
	ImGui::Text("v s: %.3f p: %.3f", velocitySetpoint, motionProfile.getVelocity());
	 */
	
	ImGui::PopID();
	
	ImGui::EndGroup();
	
	ImGui::SameLine();
	ImGui::InvisibleButton("Spacer", ImVec2(ImGui::GetTextLineHeight()*1.0, 1.0));
};

void AnimatableVelocity::settingsGui(){
	controlChannelAxis_parameter->gui(Fonts::sansBold15);
	controlChannelInvert_parameter->gui(Fonts::sansBold15);
}
