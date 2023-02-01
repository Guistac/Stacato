#include <pch.h>

#include "SharedAxisMachine.h"
#include "Motion/Axis/PositionControlledAxis.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/NodeGraph/DeviceNode.h"

#include "Environnement/Environnement.h"

/*

static bool b_invalidValue = false;
static void pushInvalidValue(bool doit){
	if(doit){
		b_invalidValue = true;
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::PushFont(Fonts::sansBold15);
	}
}

static void popInvalidValue(){
	if(b_invalidValue){
		b_invalidValue = false;
		ImGui::PopStyleColor();
		ImGui::PopFont();
	}
}

void SharedAxisMachine::controlsGui() {
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Manual Controls");
	ImGui::PopFont();
	
	widgetGui();
	
	ImGui::Text("Axis 1 Limits: %.3f - %.3f", axis1Animatable->lowerPositionLimit, axis1Animatable->upperPositionLimit);
	ImGui::Text("Axis 2 Limits: %.3f - %.3f", axis2Animatable->lowerPositionLimit, axis2Animatable->upperPositionLimit);
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis Offsets");
	ImGui::PopFont();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis 1 offset (%s)", axis1Animatable->getName());
	ImGui::PopFont();
	static char formatText[64];
	sprintf(formatText, "%.3f%s", axis1OffsetEditDisplay, positionUnit->abbreviated);
	ImGui::InputDouble("##axis1Offset", &axis1OffsetEditDisplay, 0, 0, formatText);
	ImGui::SameLine();
	if(ImGui::Button("Set Position##axis1")) captureAxis1PositionToOffset(axis1OffsetEditDisplay);
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis 2 offset (%s)", axis2Animatable->getName());
	ImGui::PopFont();
	sprintf(formatText, "%.3f%s", axis2OffsetEditDisplay, positionUnit->abbreviated);
	ImGui::InputDouble("##axis2Offset", &axis2OffsetEditDisplay, 0, 0, formatText);
	ImGui::SameLine();
	if(ImGui::Button("Set Position##axis2")) captureAxis2PositionToOffset(axis2OffsetEditDisplay);
	
	
	if(enableAntiCollision->value){
		ImGui::Separator();
		
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Anti Collision Control");
		ImGui::PopFont();
		
		ImGui::Checkbox("##DisableAnticollision", &b_disableAntiCollision);
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::TextWrapped("Disable Anti-Collision");
		ImGui::PopFont();
		ImGui::Text("Minimum Distance between axes is %.3f%s", minimumDistanceBetweenAxes->value, positionUnit->abbreviated);
		if(ImGui::Button("Capture Minimum Distance")){
			
		}
	}
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Homing");
	ImGui::PopFont();
	
	if(ImGui::Button("Start Homing")){
		
	}
	
}


void SharedAxisMachine::settingsGui() {
		
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::TextWrapped("Both axes need to have:");
	ImGui::TreePush();
	ImGui::Text("- the same position unit");
	ImGui::Text("- the same position scaling");
	ImGui::Text("- the same signed direction");
	ImGui::TreePop();
	ImGui::PopStyleColor();
	
	if(!areAxesConnected()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Axes are not connected.");
		ImGui::PopStyleColor();
		return;
	}else if(!axesHaveSamePositionUnit()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::TextWrapped("Axes don't have the same position unit.");
		ImGui::PopStyleColor();
		return;
	}
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	ImGui::Separator(); //————————————————————————
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis 1");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansLight20);
	ImGui::Text("(%s)", axis1->getName());
	ImGui::PopFont();
	
	invertAxis1->gui();
	ImGui::SameLine();
	ImGui::Text("Invert Direction");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis Offset");
	ImGui::PopFont();
	axis1Offset->gui();
	
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Axis 2");
	ImGui::PopFont();
	ImGui::SameLine();
	ImGui::PushFont(Fonts::sansLight20);
	ImGui::Text("(%s)", axis2->getName());
	ImGui::PopFont();
	
	invertAxis2->gui();
	ImGui::SameLine();
	ImGui::Text("Invert Direction");
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Axis Offset");
	ImGui::PopFont();
	axis2Offset->gui();
	
	
	ImGui::Separator(); //————————————————————————
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Motion Limits");
	ImGui::PopFont();
	
	double axisVelocityLimit = std::min(axis1->getVelocityLimit(), axis2->getVelocityLimit());
	double axisAccelerationLimit = std::min(axis1->getAccelerationLimit(), axis2->getAccelerationLimit());
	
	ImGui::PushStyleColor(ImGuiCol_Text, Colors::gray);
	ImGui::Text("Axes Limit Velocity to %.3f %s/s", axisVelocityLimit, axis1->getPositionUnit()->abbreviated);
	ImGui::Text("Axes Limit Acceleration to %.3f %s/s2", axisAccelerationLimit, axis1->getPositionUnit()->abbreviated);
	ImGui::PopStyleColor();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Velocity Limit");
	ImGui::PopFont();
	velocityLimit->gui();
	
	ImGui::PushFont(Fonts::sansBold15);
	ImGui::Text("Acceleration Limit");
	ImGui::PopFont();
	accelerationLimit->gui();
		
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Collision Avoidance");
	ImGui::PopFont();
	
	enableAntiCollision->gui();
	ImGui::SameLine();
	ImGui::Text("Enable Collision Avoidance");
	if(enableAntiCollision->value){
		axis1isAboveAxis2->gui();
		ImGui::SameLine();
		ImGui::TextWrapped("Axis Order: %s", getAxis1()->getName());
		ImGui::SameLine();
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("%s", axis1isAboveAxis2->value ? ">" : "<");
		ImGui::PopFont();
		ImGui::SameLine();
		ImGui::TextWrapped("%s", getAxis2()->getName());
		
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Minimum Distance Between Axes");
		ImGui::PopFont();
		minimumDistanceBetweenAxes->gui();
	}
	
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("Synchronous Control");
	ImGui::PopFont();
	
	enableSynchronousControl->gui();
	ImGui::SameLine();
	ImGui::Text("Enable Synchronous Control");
	
	if(enableSynchronousControl->value){
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::Text("Synchronisation master selection");
		ImGui::PopFont();
		axis1isMaster->gui();
		ImGui::SameLine();
		ImGui::Text("%s is Master", axis1isMaster->value ? axis1->getName() : axis2->getName());
	}
	
	ImGui::Separator();
	
	if(positionUnit->unitType == Units::Type::LINEAR_DISTANCE){
	
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("Control Widget");
		ImGui::PopFont();
		
		
		horizontalControls->gui();
		ImGui::SameLine();
		ImGui::TextWrapped("Horizontal Manual Controls");
		
	}
	
	
	ImGui::Separator(); //————————————————————————
	
	
	ImGui::PushFont(Fonts::sansBold20);
	ImGui::Text("User Setup");
	ImGui::PopFont();
	
	allowUserHoming->gui();
	ImGui::SameLine();
	ImGui::Text("Allow User Homing");
	
}

void SharedAxisMachine::axisGui() {
	if (!areAxesConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}

	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	if(ImGui::BeginTabBar("AxesTabBar")){
		
		if(ImGui::BeginTabItem(axis1->getName())){
			if (ImGui::BeginTabBar("AxisTabBar")) {
				axis1->nodeSpecificGui();
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		
		if(ImGui::BeginTabItem(axis2->getName())){
			if (ImGui::BeginTabBar("AxisTabBar")) {
				axis2->nodeSpecificGui();
				ImGui::EndTabBar();
			}
			ImGui::EndTabItem();
		}
		
		ImGui::EndTabBar();
	}

}

void SharedAxisMachine::deviceGui() {
	std::vector<std::shared_ptr<Device>> devices;
	getDevices(devices);

	auto deviceTabBar = [](std::shared_ptr<Device> device) {
		ImGui::PushFont(Fonts::sansBold20);
		ImGui::Text("%s", device->getName());
		ImGui::PopFont();
		if (ImGui::BeginTabBar(device->getName())) {
			device->nodeSpecificGui();
			ImGui::EndTabBar();
		}
	};

	if (devices.size() == 1) deviceTabBar(devices.front());
	else if (!devices.empty()) {
		if (ImGui::BeginTabBar("DevicesTabBar")) {
			for (auto& device : devices) {
				if (ImGui::BeginTabItem(device->getName())) {
					deviceTabBar(device);
					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();
		}
	}
}

void SharedAxisMachine::metricsGui() {
	if (!isAxisConnected()) {
		ImGui::Text("No Axis Connected");
		return;
	}
	std::shared_ptr<PositionControlledAxis> axis = getAxis();
	axis->metricsGui();
}





void SharedAxisMachine::ControlWidget::gui(){
	//draw the header later, we don't know the size of the widget in advance
	glm::vec2 headerCursor = machine->reserveSpaceForMachineHeaderGui();

	//early out if we don't have both axes
	if(!machine->areAxesConnected()) {
		machine->machineHeaderGui(headerCursor);
		ImGui::Text("Axes are not connected.");
		return;
	}

	machine->widgetGui();

	//we now know the width of the widget, we can draw the header centered
	//also draw the state control widget
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursor, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
	
}

void SharedAxisMachine::widgetGui(){
	
	//Begin main widget content group
	ImGui::BeginGroup();
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));
	ImGui::BeginDisabled(!isEnabled());
	
	
	float sliderHeight = ImGui::GetTextLineHeight() * 10.0;
	double separatorWidth = ImGui::GetTextLineHeight() * .25f;
    
	float minPos = std::min(axis1Animatable->lowerPositionLimit, axis2Animatable->lowerPositionLimit);
	float maxPos = std::max(axis1Animatable->upperPositionLimit, axis2Animatable->upperPositionLimit);
	
	auto getNormalizedPosition = [minPos, maxPos](double pos) -> double {
		double norm = (pos - minPos) / (maxPos - minPos);
		return std::clamp(norm, 0.0, 1.0);
	};
	
	auto getNormalizedDistance = [minPos, maxPos](double distance) -> double{
		if(maxPos == minPos) return 0.0;
		return std::abs(distance / (maxPos - minPos));
	};
	
	//Draw Visualizer (Angular or Linear)
	if(positionUnit->unitType == Units::Type::LINEAR_DISTANCE){
		
		float feedbackWidth = ImGui::GetTextLineHeight() * 2.f;
		
		if(positionUnit->unitType == Units::Type::LINEAR_DISTANCE && horizontalControls->value){
					
			//draw control sliders and widgets
			axis1Animatable->manualControlsHorizontalGui(sliderHeight, axis1Animatable->getName());
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + separatorWidth);
			axis2Animatable->manualControlsHorizontalGui(sliderHeight, axis2Animatable->getName());
			if(enableSynchronousControl->value){
				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + separatorWidth);
				synchronizedAnimatable->manualControlsHorizontalGui(sliderHeight, "Synchro");
			}
			ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y + separatorWidth);
			float controlsWidth = ImGui::GetItemRectSize().x;
			
			ImGui::BeginGroup();
			ImGui::InvisibleButton("LinearAxisDisplay", glm::vec2(controlsWidth, feedbackWidth));
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			glm::vec2 size = max - min;
			float middleY = min.y + size.y * .5f;
						
			//position indicator background
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			ImGui::PushClipRect(min, max, true);
			
			if(getState() == DeviceState::OFFLINE){
				drawing->AddRectFilled(min, max, ImColor(Colors::blue));
			}else{
				
				drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));
				
				//draw keepout constraints
				for(auto& constraint : axis1Animatable->getConstraints()){
					if(constraint->getType() != AnimationConstraint::Type::KEEPOUT) continue;
					auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
					double constraintMinX = min.x + size.x * getNormalizedPosition(keepout->keepOutMinPosition);
					double constraintMaxX = min.x + size.x * getNormalizedPosition(keepout->keepOutMaxPosition);
					drawing->AddRectFilled(glm::vec2(constraintMinX, min.y),
										   glm::vec2(constraintMaxX, middleY),
										   constraint->isEnabled() ? ImColor(1.f, 0.f, 0.f, .4f) : ImColor(1.f, 1.f, 1.f, .2f));
				}
				for(auto& constraint : axis2Animatable->getConstraints()){
					if(constraint->getType() != AnimationConstraint::Type::KEEPOUT) continue;
					auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
					double constraintMinX = min.x + size.x * getNormalizedPosition(keepout->keepOutMinPosition);
					double constraintMaxX = min.x + size.x * getNormalizedPosition(keepout->keepOutMaxPosition);
					drawing->AddRectFilled(glm::vec2(constraintMinX, middleY),
										   glm::vec2(constraintMaxX, max.y),
										   constraint->isEnabled() ? ImColor(1.f, 0.f, 0.f, .4f) : ImColor(1.f, 1.f, 1.f, .2f));
				}
				
				//draw constraint limit lines
				ImColor limitLineColor = ImColor(.0f, .0f, .0f, 1.f);
				float limitLineThickness = ImGui::GetTextLineHeight() * .05f;
				
				double axis1ConstraintMin, axis1ConstraintMax;
				axis1Animatable->getConstraintPositionLimits(axis1ConstraintMin, axis1ConstraintMax);
				double axis1ConstraintMinX = min.x + size.x * getNormalizedPosition(axis1ConstraintMin);
				double axis1ConstraintMaxX = min.x + size.x * getNormalizedPosition(axis1ConstraintMax);
				drawing->AddLine(glm::vec2(axis1ConstraintMinX, min.y),
								 glm::vec2(axis1ConstraintMinX, middleY),
								 limitLineColor, limitLineThickness);
				drawing->AddLine(glm::vec2(axis1ConstraintMaxX, min.y),
								 glm::vec2(axis1ConstraintMaxX, middleY),
								 limitLineColor, limitLineThickness);
				
				double axis2ConstraintMin, axis2ConstraintMax;
				axis2Animatable->getConstraintPositionLimits(axis2ConstraintMin, axis2ConstraintMax);
				double axis2ConstraintMinX = min.x + size.x * getNormalizedPosition(axis2ConstraintMin);
				double axis2ConstraintMaxX = min.x + size.x * getNormalizedPosition(axis2ConstraintMax);
				drawing->AddLine(glm::vec2(axis2ConstraintMinX, middleY),
								 glm::vec2(axis2ConstraintMinX, max.y),
								 limitLineColor, limitLineThickness);
				drawing->AddLine(glm::vec2(axis2ConstraintMaxX, middleY),
								 glm::vec2(axis2ConstraintMaxX, max.y),
								 limitLineColor, limitLineThickness);
				
				
				
				//draw rapid target
				
				ImColor targetColor = ImColor(Colors::yellow);
				float targetLineThickness = ImGui::GetTextLineHeight() * .05f;
				float targetTriangleSize = ImGui::GetTextLineHeight() * .4f;
				
				if(axis1Animatable->isInRapid()){
					float targetX = min.x + size.x * getNormalizedPosition(axis1Animatable->getRapidTarget()->toPosition()->position);
					float triangleY = min.y + size.y * .25f;
					drawing->AddLine(glm::vec2(targetX, min.y), glm::vec2(targetX, middleY), targetColor, targetLineThickness);
					drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
											   glm::vec2(targetX - targetTriangleSize, triangleY + targetTriangleSize * .4f),
											   glm::vec2(targetX - targetTriangleSize, triangleY - targetTriangleSize * .4f),
											   targetColor);
					drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
											   glm::vec2(targetX + targetTriangleSize, triangleY - targetTriangleSize * .4f),
											   glm::vec2(targetX + targetTriangleSize, triangleY + targetTriangleSize * .4f),
											   targetColor);
				}
				if(axis2Animatable->isInRapid()){
					float targetX = min.x + size.x * getNormalizedPosition(axis2Animatable->getRapidTarget()->toPosition()->position);
					float triangleY = middleY + size.y * .25f;
					drawing->AddLine(glm::vec2(targetX, middleY), glm::vec2(targetX, max.y), targetColor, targetLineThickness);
					drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
											   glm::vec2(targetX - targetTriangleSize, triangleY + targetTriangleSize * .4f),
											   glm::vec2(targetX - targetTriangleSize, triangleY - targetTriangleSize * .4f),
											   targetColor);
					drawing->AddTriangleFilled(glm::vec2(targetX, triangleY),
											   glm::vec2(targetX + targetTriangleSize, triangleY - targetTriangleSize * .4f),
											   glm::vec2(targetX + targetTriangleSize, triangleY + targetTriangleSize * .4f),
											   targetColor);
				}
				
				
				
				//draw position arrow indicators
				float lineThickness = ImGui::GetTextLineHeight() * .05f;
				float triangleSize = ImGui::GetTextLineHeight() * .5f;
				ImColor positionIndicatorColor = ImColor(Colors::white);
				ImColor brakingPositionIndicatorColor = ImColor(1.f, 1.f, 1.f, .3f);
				
				double axis1_X = min.x + size.x * getNormalizedPosition(axis1Animatable->getActualPosition());
				double axis2_X = min.x + size.x * getNormalizedPosition(axis2Animatable->getActualPosition());
				double axis1BrakingPositionX = min.x + size.x * getNormalizedPosition(axis1Animatable->getBrakingPosition());
				double axis2BrakingPositionX = min.x + size.x * getNormalizedPosition(axis2Animatable->getBrakingPosition());
				
				drawing->AddLine(glm::vec2(axis1BrakingPositionX, min.y),
								 glm::vec2(axis1BrakingPositionX, middleY - triangleSize + 1.f),
								 brakingPositionIndicatorColor, lineThickness);
				drawing->AddLine(glm::vec2(axis2BrakingPositionX, max.y),
								 glm::vec2(axis2BrakingPositionX, middleY + triangleSize - 1.f),
								 brakingPositionIndicatorColor, lineThickness);
				drawing->AddTriangleFilled(glm::vec2(axis1BrakingPositionX, middleY),
										   glm::vec2(axis1BrakingPositionX - triangleSize * .4f, middleY - triangleSize),
										   glm::vec2(axis1BrakingPositionX + triangleSize * .4f, middleY - triangleSize),
										   brakingPositionIndicatorColor);
				drawing->AddTriangleFilled(glm::vec2(axis2BrakingPositionX, middleY),
										   glm::vec2(axis2BrakingPositionX + triangleSize * .4f, middleY + triangleSize),
										   glm::vec2(axis2BrakingPositionX - triangleSize * .4f, middleY + triangleSize),
										   brakingPositionIndicatorColor);
				
				drawing->AddLine(glm::vec2(axis1_X, min.y), glm::vec2(axis1_X, middleY - triangleSize + 1.f), positionIndicatorColor, lineThickness);
				drawing->AddLine(glm::vec2(axis2_X, max.y), glm::vec2(axis2_X, middleY + triangleSize - 1.f), positionIndicatorColor, lineThickness);
				drawing->AddTriangleFilled(glm::vec2(axis1_X, middleY),
										   glm::vec2(axis1_X - triangleSize * .4f, middleY - triangleSize),
										   glm::vec2(axis1_X + triangleSize * .4f, middleY - triangleSize),
										   positionIndicatorColor);
				drawing->AddTriangleFilled(glm::vec2(axis2_X, middleY),
										   glm::vec2(axis2_X + triangleSize * .4f, middleY + triangleSize),
										   glm::vec2(axis2_X - triangleSize * .4f, middleY + triangleSize),
										   positionIndicatorColor);
			}
			
			//draw separator and frame lines
			drawing->AddLine(glm::vec2(min.x, middleY), glm::vec2(max.x, middleY), ImColor(Colors::veryDarkGray), ImGui::GetTextLineHeight() * .025f);
			float frameWidth = ImGui::GetTextLineHeight() * 0.05;
			drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);
			
			ImGui::PopClipRect();
			
		}else{
			
			axis1Animatable->manualControlsVerticalGui(sliderHeight, axis1Animatable->getName());
			ImGui::SameLine(.0f, separatorWidth);
			axis2Animatable->manualControlsVerticalGui(sliderHeight, axis2Animatable->getName());
			if(enableSynchronousControl->value){
				ImGui::SameLine(.0f, separatorWidth);
				synchronizedAnimatable->manualControlsVerticalGui(sliderHeight, "Synchro");
			}
			
			ImGui::SameLine(.0f, separatorWidth);
			ImGui::BeginGroup();
			
			ImGui::InvisibleButton("LinearAxisDisplay", glm::vec2(feedbackWidth, sliderHeight));
			glm::vec2 min = ImGui::GetItemRectMin();
			glm::vec2 max = ImGui::GetItemRectMax();
			glm::vec2 size = max - min;
			ImDrawList* drawing = ImGui::GetWindowDrawList();
			
			if(getState() == DeviceState::OFFLINE){
				drawing->AddRectFilled(min, max, ImColor(Colors::blue));
			}else{
				drawing->AddRectFilled(min, max, ImColor(Colors::gray));
				//TODO: draw vertical machine feedback
			}
			
		}
		
		
		
		
	}else if(positionUnit->unitType == Units::Type::ANGULAR_DISTANCE){
		
		ImGui::BeginDisabled(controlMode == ControlMode::SYNCHRONOUS);
		axis1Animatable->manualControlsVerticalGui(sliderHeight, axis1Animatable->getName());
		ImGui::SameLine(.0f, separatorWidth);
		axis2Animatable->manualControlsVerticalGui(sliderHeight, axis2Animatable->getName());
		ImGui::EndDisabled();
		
		if(enableSynchronousControl->value){
			ImGui::SameLine(.0f, separatorWidth);
			ImGui::BeginDisabled(controlMode == ControlMode::INDIVIDUAL);
			synchronizedAnimatable->manualControlsVerticalGui(sliderHeight, "Synchro");
			ImGui::EndDisabled();
		}
		
		ImGui::SameLine(.0f, separatorWidth);
		ImGui::BeginGroup();
		
		float displayDiameter = sliderHeight;
		
		//——— title card
		ImGui::PushFont(Fonts::sansRegular12);
		glm::vec2 nameFrameSize(displayDiameter, ImGui::GetTextLineHeight());
		backgroundText("Feedback", nameFrameSize, Colors::darkGray);
		ImGui::PopFont();
		
		ImGui::InvisibleButton("rotatingAxisDisplay", glm::vec2(displayDiameter));
		glm::vec2 min = ImGui::GetItemRectMin();
		glm::vec2 max = ImGui::GetItemRectMax();
		glm::vec2 size = max - min;
		glm::vec2 middle = (max + min) / 2.0;
		float radius = displayDiameter / 2.0;
					
		float triangleSize = ImGui::GetTextLineHeight() * .5f;
		float lineWidth = ImGui::GetTextLineHeight() * .1f;
		
		ImDrawList* drawing = ImGui::GetWindowDrawList();
		
		auto drawArrowRotated = [&](glm::vec2 center, float startRadius, float endRadius, float angleRadians){
			float lineEndRadius = endRadius - triangleSize * .5f;
			glm::vec2 start(startRadius * std::cos(angleRadians), startRadius * std::sin(angleRadians));
			glm::vec2 end(lineEndRadius * std::cos(angleRadians), lineEndRadius * std::sin(angleRadians));
			start += center;
			end += center;
			
			glm::vec2 trianglePoints[3] = {
				center + glm::rotate(glm::vec2(endRadius, 0), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, triangleSize * .4f), angleRadians),
				center + glm::rotate(glm::vec2(endRadius - triangleSize, -triangleSize * .4f), angleRadians)
			};
			
			drawing->AddLine(start, end, ImColor(Colors::white), lineWidth);
			drawing->AddTriangleFilled(trianglePoints[0],
								 trianglePoints[1],
								 trianglePoints[2],
								 ImColor(Colors::white));
			
		};
		
		
		if(getState() == DeviceState::OFFLINE){
			drawing->AddCircleFilled(middle, size.x * .5f, ImColor(Colors::blue));
		}else{
			
			drawing->AddCircleFilled(middle, size.x / 2.0, ImColor(Colors::darkGray));
			
			ImColor borderColor = ImColor(Colors::black);
			float borderWidth = ImGui::GetTextLineHeight() * .05f;

			//get visualizer angles
			double angle1 = axis2Animatable->getActualPosition();
			double angle2 = axis1Animatable->getActualPosition();
			double displayAngle1 = Units::convert(angle1, positionUnit, Units::AngularDistance::Radian) - M_PI_2;
			double displayAngle2 = Units::convert(angle2, positionUnit, Units::AngularDistance::Radian) - M_PI_2;
			
			//draw visualizer arrows
			drawing->AddCircle(middle, radius, borderColor, 64, borderWidth);
			drawArrowRotated(middle, radius * .5f, radius, displayAngle1);
			drawing->AddCircle(middle, radius * .5f, borderColor, 64, borderWidth);
			drawArrowRotated(middle, 0.0, radius * .5f, displayAngle2);
			drawing->AddCircleFilled(middle, lineWidth, borderColor, 64);

			//zero tick mark
			float zeroTickLength = ImGui::GetTextLineHeight() * .25f;
			drawing->AddLine(middle, middle + glm::vec2(0, -radius), ImColor(0.f, 0.f, 0.f, .2f), borderWidth);
			drawing->AddLine(middle + glm::vec2(0, -radius), middle + glm::vec2(0, -radius + zeroTickLength), ImColor(Colors::white), borderWidth);
			drawing->AddLine(middle + glm::vec2(0, -radius * .5f), middle + glm::vec2(0, -radius * .5f + zeroTickLength), ImColor(Colors::white), borderWidth);
			
			
			//display position string on turntables
			static char angle1String[64];
			static char angle2String[64];
			if(positionUnit == Units::AngularDistance::Degree){
				auto getDegreeString = [](char* output, double angleDegrees){
					int extraRevs = floor(std::abs(angleDegrees) / 360.0);
					if(angleDegrees < 0.0) extraRevs++;
					float angleSingleTurn;
					if(angleDegrees >= 0.0) angleSingleTurn = fmod(angleDegrees, 360.0);
					else angleSingleTurn = 360.0 + fmod(angleDegrees, 360.0);
					sprintf(output, "%.1f%s%s%i", angleSingleTurn,  Units::AngularDistance::Degree->abbreviated, angleDegrees > 0.0 ? "+" : "-", extraRevs);
				};
				getDegreeString(angle1String, angle1);
				getDegreeString(angle2String, angle2);
			}else if(positionUnit == Units::AngularDistance::Radian){
				sprintf(angle1String, "%.3f%s", fmod(angle1, 2.0 * M_PI), positionUnit->abbreviated);
				sprintf(angle2String, "%.3f%s", fmod(angle2, 2.0 * M_PI), positionUnit->abbreviated);
			}else{
				sprintf(angle1String, "%.3f%s", angle1, positionUnit->abbreviated);
				sprintf(angle2String, "%.3f%s", angle2, positionUnit->abbreviated);
			}
			ImGui::PushFont(Fonts::sansBold15);
			glm::vec2 textSize;
			glm::vec2 textPos;
			textSize = ImGui::CalcTextSize(angle1String);
			textPos = middle + glm::vec2(0, - 3.0 * radius / 4.0) - textSize / 2.0;
			drawing->AddText(textPos, ImColor(0.f, 0.f, 0.f, .6f), angle1String);
			textSize = ImGui::CalcTextSize(angle2String);
			textPos = middle + glm::vec2(0, -radius / 4.0) - textSize / 2.0;
			drawing->AddText(textPos, ImColor(0.f, 0.f, 0.f, .6f), angle2String);
			ImGui::PopFont();
			
			glm::vec2 rangeDisplaySize(radius * 2.0, ImGui::GetTextLineHeight());
			static char rangeString[32];
			float rangeProgress = axis1Animatable->getActualPositionNormalized();
			sprintf(rangeString, "%s : %.1f%%", axis1Animatable->getName(), rangeProgress * 100.0);
			ImGui::ProgressBar(rangeProgress, rangeDisplaySize, rangeString);
			rangeProgress = axis2Animatable->getActualPositionNormalized();
			sprintf(rangeString, "%s : %.1f%%", axis2Animatable->getName(), rangeProgress * 100.0);
			ImGui::ProgressBar(rangeProgress, rangeDisplaySize, rangeString);
				
		}
		
        
		ImGui::Checkbox("##forceSync", &b_forceSynchronousControl);
		ImGui::SameLine();
		ImGui::Text("Force Sync");
		
		if(controlMode == ControlMode::NONE) ImGui::Text("None");
		else if(controlMode == ControlMode::SYNCHRONOUS) ImGui::Text("Synchronous");
		else if(controlMode == ControlMode::INDIVIDUAL) ImGui::Text("Individual");
		 
	}
	ImGui::EndGroup();
	
	//end main widget content group
	ImGui::EndGroup();
	ImGui::EndDisabled();
	ImGui::PopStyleVar();
}


bool SharedAxisMachine::hasSetupGui(){
	return allowUserHoming->value;
}

void SharedAxisMachine::setupGui(){
	
	if(!areAxesConnected()){
		ImGui::PushStyleColor(ImGuiCol_Text, Colors::red);
		ImGui::Text("Axes are not connected");
		ImGui::PopStyleColor();
		return;
	}
	
	auto axis1 = getAxis1();
	auto axis2 = getAxis2();
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetStyle().ItemSpacing.y));
	
	if(allowUserHoming->value){
		
		glm::vec2 homingButtonSize(ImGui::GetTextLineHeight() * 10.0, ImGui::GetFrameHeight());
		glm::vec2 homingIndicatorSize(ImGui::GetTextLineHeight() * 10.0, ImGui::GetFrameHeight());
		
		char homingButtonString[64];
		
		ImGui::BeginDisabled(!canStartHoming());
		
		auto homingGui = [&](std::shared_ptr<PositionControlledAxis> axis){
			if(axis->isHoming()){
				sprintf(homingButtonString, "Stop Homing %s", axis->getName());
				ImGui::PushStyleColor(ImGuiCol_Button, Colors::green);
				if(ImGui::Button(homingButtonString, homingButtonSize)) axis->cancelHoming();
				ImGui::PopStyleColor();
			}else{
				sprintf(homingButtonString, "Start Homing %s", axis->getName());
				if(ImGui::Button(homingButtonString, homingButtonSize)) axis->startHoming();
			}
			ImGui::SameLine();
			ImVec4 axisHomingProgressIndicatorColor = Colors::darkGray;
			if(axis->isHoming()) axisHomingProgressIndicatorColor = Colors::orange;
			else if(axis->didHomingSucceed()) axisHomingProgressIndicatorColor = Colors::green;
			else if(axis->didHomingFail()) axisHomingProgressIndicatorColor = Colors::red;
			backgroundText(Enumerator::getDisplayString(axis->getHomingStep()), homingIndicatorSize, axisHomingProgressIndicatorColor);
		};
		
		if(ImGui::Button("Start Homing Both Axes", homingButtonSize)) startHomingBothAxes();
		ImGui::SameLine();
		if(ImGui::Button("Stop Homing All Axes", homingButtonSize)) {
			axis1->cancelHoming();
			axis2->cancelHoming();
		}
		homingGui(axis1);
		homingGui(axis2);
		
		ImGui::EndDisabled();
		
	}
	
	ImGui::PopStyleVar();
	
}

*/
