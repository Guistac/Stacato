#include <pch.h>

#include "MultipointHoist.h"

#include "Motion/Axis/AxisNode.h"

#include <imgui.h>
#include <imgui_internal.h>

#include "Gui/Assets/Fonts.h"
#include "Gui/Assets/Colors.h"

#include "Gui/Utilities/CustomWidgets.h"

#include "Environnement/NodeGraph/DeviceNode.h"

#include "Environnement/Environnement.h"


void MultipointHoist::controlsGui() {
	widgetGui();
}

void MultipointHoist::settingsGui() {
	if(ImGui::Button("Add Axis")) addAxisMapping();
	
	std::shared_ptr<AxisMapping> deletedMapping = nullptr;
	
	for(int i = 0; i < axisMappings.size(); i++){
		
		ImGui::PushID(i);
		
		auto axisMapping = axisMappings[i];
		
		ImGui::PushFont(Fonts::sansBold20);
		if(buttonCross("##delete", ImGui::GetTextLineHeight())) deletedMapping = axisMapping;
		ImGui::PopFont();
		
		ImGui::SameLine();
		
		if(!axisMapping->axisPin->isConnected()){
			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Axis %i : No Axis Connected", i);
			ImGui::PopFont();
			
		}
		else{
			
			auto axis = axisMapping->axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
			auto animatable = axisMapping->animatablePosition;
			
			ImGui::PushFont(Fonts::sansBold20);
			ImGui::Text("Axis %i : %s", i, axis->getName().c_str());
			ImGui::PopFont();
			
			if(ImGui::BeginTable("##axisProperties", 2, ImGuiTableFlags_Borders)){
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Lower Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", axis->getLowerPositionLimit(), axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Upper Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", axis->getUpperPositionLimit(), axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Velocity Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s", axis->getVelocityLimit(), axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Acceleration Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s\xc2\xb2", axis->getAccelerationLimit(), axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();

				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Lower Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", animatable->lowerPositionLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Upper Position Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s", animatable->upperPositionLimit, axis->getPositionUnit()->abbreviated);
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Velocity Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s", animatable->velocityLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();
				
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Animatable Acceleration Limit");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("%.3f%s/s\xc2\xb2", animatable->accelerationLimit, axis->getPositionUnit()->abbreviated);
				ImGui::TableNextRow();

				
				
				ImGui::EndTable();
			}
			
			if(ImGui::Checkbox("Invert Direction", &axisMapping->b_invertDirection)){
				axisMapping->updateAnimatableParameters();
			}
			if(ImGui::InputDouble("Position Offset", &axisMapping->positionOffset)){
				axisMapping->updateAnimatableParameters();
			}
			
		}
		
		ImGui::PopID();
		
	}
	if(deletedMapping) removeAxisMapping(deletedMapping);
	
	
	
	
	ImGui::Separator();
	
	if(axisMappings.size() == 2){
		ImGui::Checkbox("Enable Two Axis Distance Constraint", &b_enableTwoAxisDistanceConstraint);
		ImGui::BeginDisabled(!b_enableTwoAxisDistanceConstraint);
		ImGui::InputDouble("Max Distance Between Axes", &maxDistanceBetweenAxes);
		ImGui::EndDisabled();
	}
	
}



void MultipointHoist::axisGui() {}
void MultipointHoist::deviceGui() {}
void MultipointHoist::metricsGui() {}



void MultipointHoist::AxisMapping::controlGui(){
	
	ImGui::BeginGroup();
		
	std::shared_ptr<AxisInterface> axis = nullptr;
	bool b_disableControlGui = true;
	if(axisPin->isConnected()) {
		axis = axisPin->getConnectedPin()->getSharedPointer<AxisInterface>();
		b_disableControlGui = axis->getState() != DeviceState::ENABLED;
	}
	bool b_invertControlGui = false;
	
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.2));

	ImGui::BeginDisabled(b_disableControlGui);
	animatablePosition->manualControlsVerticalGui(ImGui::GetTextLineHeight() * 10.f, nullptr, b_invertControlGui);
	ImGui::EndDisabled();
	float controlsHeight = ImGui::GetItemRectSize().y;

	ImGui::SameLine();

	float actualEffort = 0.0;
	if(axis) actualEffort = axis->getEffortActual();
	float effortProgress = actualEffort;
	while(effortProgress > 1.0) effortProgress -= 1.0;
	if(actualEffort > 2.0) {
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::red);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::yellow);
	}else if(actualEffort > 1.0){
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::yellow);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, Colors::green);
	}else{
		ImGui::PushStyleColor(ImGuiCol_PlotHistogram, Colors::green);
		ImGui::PushStyleColor(ImGuiCol_FrameBg, ImGui::GetColorU32(ImGuiCol_FrameBg));
	}

	verticalProgressBar(effortProgress, ImVec2(ImGui::GetTextLineHeight() * .5f, controlsHeight));
	ImGui::PopStyleColor(2);





	ImGui::SameLine();

	//draw feedback background
	float feedbackWidth = ImGui::GetTextLineHeight() * 3.0;
	ImGui::InvisibleButton("feedback", glm::vec2(feedbackWidth, controlsHeight));
	glm::vec2 min = ImGui::GetItemRectMin();
	glm::vec2 max = ImGui::GetItemRectMax();
	glm::vec2 size = max - min;
	ImDrawList* drawing = ImGui::GetWindowDrawList();

	ImGui::PushClipRect(min, max, true);

	if(axis == nullptr || axis->getState() == DeviceState::OFFLINE){
		drawing->AddRectFilled(min, max, ImColor(Colors::blue));
	}else{

	drawing->AddRectFilled(min, max, ImColor(Colors::darkGray));

	auto getDrawingPosition = [&](double position) -> double{
		if(!b_invertControlGui){
			float y = min.y + size.y - animatablePosition->normalizePosition(position) * size.y;
			return std::clamp(y, min.y, max.y);
		}else{
			float y = min.y + animatablePosition->normalizePosition(position) * size.y;
			return std::clamp(y, min.y, max.y);
		}
	};

	glm::vec2 mouse = ImGui::GetMousePos();

	//draw keepout constraints
	int keepoutConstraintCount = 0;
	//count constraints to draw them side by side instead of stacking them
	for(auto& constraint : animatablePosition->getConstraints()){
		if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
	}
	float keepoutConstraintWidth = size.x / keepoutConstraintCount;
	keepoutConstraintCount = 0;
	for(auto& constraint : animatablePosition->getConstraints()){
		int constraintXMin = std::round(min.x + keepoutConstraintCount * keepoutConstraintWidth);
		if(constraint->getType() == AnimationConstraint::Type::KEEPOUT) keepoutConstraintCount++;
		int constraintXMax = std::round(min.x + keepoutConstraintCount * keepoutConstraintWidth);
		auto keepout = std::static_pointer_cast<AnimatablePosition_KeepoutConstraint>(constraint);
		
		double minKeepout = getDrawingPosition(keepout->keepOutMinPosition);
		double maxKeepout = getDrawingPosition(keepout->keepOutMaxPosition);
		
		glm::vec2 keepoutStartPos(constraintXMin, minKeepout);
		glm::vec2 keepoutEndPos(constraintXMax, maxKeepout);
		ImColor constraintColor;
		if(!constraint->isEnabled()) constraintColor = ImColor(1.f, 1.f, 1.f, .2f);
		else constraintColor = ImColor(1.f, 0.f, 0.f, .4f);
		drawing->AddRectFilled(keepoutStartPos, keepoutEndPos, constraintColor);
		
		
		if(minKeepout > maxKeepout){
			std::swap(keepoutStartPos.y, keepoutEndPos.y);
		}
		
		if(mouse.x > keepoutStartPos.x && mouse.y > keepoutStartPos.y && mouse.x < keepoutEndPos.x && mouse.y < keepoutEndPos.y){
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, glm::vec2(0));
			ImGui::BeginTooltip();
			ImGui::Text("%s", keepout->getName().c_str());
			ImGui::EndTooltip();
			ImGui::PopStyleVar();
		}
		
		
	}

	float lineThickness = ImGui::GetTextLineHeight() * .15f;
	//draw rapid target
	if(animatablePosition->isInRapid()){
		float rapidTarget = animatablePosition->getRapidTarget()->toPosition()->position;
		
		float rapidTargetY = getDrawingPosition(rapidTarget);
		
		ImColor targetColor = ImColor(Colors::yellow);
		drawing->AddLine(glm::vec2(min.x, rapidTargetY), glm::vec2(max.x, rapidTargetY), targetColor, lineThickness);
		float middleX = min.x + size.x * .5f;
		float triangleSize = ImGui::GetTextLineHeight() * .4f;
		drawing->AddTriangleFilled(glm::vec2(middleX, rapidTargetY),
								   glm::vec2(middleX - triangleSize * .4f, rapidTargetY - triangleSize),
								   glm::vec2(middleX + triangleSize * .4f, rapidTargetY - triangleSize),
								   targetColor);
		drawing->AddTriangleFilled(glm::vec2(middleX, rapidTargetY),
								   glm::vec2(middleX + triangleSize * .4f, rapidTargetY + triangleSize),
								   glm::vec2(middleX - triangleSize * .4f, rapidTargetY + triangleSize),
								   targetColor);
	}

	//draw braking position
	float brakingPositionY = getDrawingPosition(animatablePosition->getBrakingPosition());
	float triangleSize = ImGui::GetTextLineHeight() * .75f;
	ImColor brakingPositionColor = ImColor(1.f, 1.f, 1.f, .3f);
	drawing->AddLine(glm::vec2(min.x, brakingPositionY), glm::vec2(max.x - triangleSize, brakingPositionY), brakingPositionColor, lineThickness);
	drawing->AddTriangleFilled(glm::vec2(max.x, brakingPositionY),
							   glm::vec2(max.x - triangleSize, brakingPositionY + triangleSize * .4f),
							   glm::vec2(max.x - triangleSize, brakingPositionY - triangleSize * .4f),
							   brakingPositionColor);

	//draw actual position
	float axisPositionY = getDrawingPosition(animatablePosition->getActualPosition());
	drawing->AddLine(glm::vec2(min.x, axisPositionY), glm::vec2(max.x - triangleSize + 1.f, axisPositionY), ImColor(Colors::white), lineThickness);
	drawing->AddTriangleFilled(glm::vec2(max.x, axisPositionY),
							   glm::vec2(max.x - triangleSize, axisPositionY + triangleSize * .4f),
							   glm::vec2(max.x - triangleSize, axisPositionY - triangleSize * .4f),
							   ImColor(Colors::white));

	//draw current constraint limits
	double minPositionLimit, maxPositionLimit;
	animatablePosition->getConstraintPositionLimits(minPositionLimit, maxPositionLimit);

	double minPosY = getDrawingPosition(minPositionLimit);
	double maxPosY = getDrawingPosition(maxPositionLimit);



	ImColor limitLineColor = ImColor(0.f, 0.f, 0.f, .4f);
	float limitLineThickness = ImGui::GetTextLineHeight() * .05f;
	drawing->AddLine(ImVec2(min.x, minPosY), ImVec2(max.x, minPosY), limitLineColor, limitLineThickness);
	drawing->AddLine(ImVec2(min.x, maxPosY), ImVec2(max.x, maxPosY), limitLineColor, limitLineThickness);

	}

	ImGui::PopClipRect();

	//draw frame contour
	float frameWidth = ImGui::GetTextLineHeight() * 0.1f;
	drawing->AddRect(min - glm::vec2(frameWidth * .5f), max + glm::vec2(frameWidth * .5f), ImColor(Colors::black), frameWidth, ImDrawFlags_RoundCornersAll, frameWidth);

	
	ImGui::PopStyleVar();
	
	ImGui::EndGroup();
}


void MultipointHoist::ControlWidget::gui(){
	
	glm::vec2 headerCursorPos = machine->reserveSpaceForMachineHeaderGui();
	
	ImGui::BeginGroup();
	machine->widgetGui();
	ImGui::EndGroup();
	
	float widgetWidth = ImGui::GetItemRectSize().x;
	machine->machineHeaderGui(headerCursorPos, widgetWidth);
	machine->machineStateControlGui(widgetWidth);
	
}

void MultipointHoist::widgetGui(){
	float controlsHeight = ImGui::GetTextLineHeight() * 10.0;
	
	for(int i = 0; i < axisMappings.size(); i++){
		auto mapping = axisMappings[i];
		auto animatable = mapping->animatablePosition;
		bool b_axisConnected = mapping->axisPin->isConnected();
		
		ImGui::PushID(i);
		ImGui::BeginGroup();
		
		ImGui::BeginDisabled(!b_axisConnected);
		mapping->controlGui();
		ImGui::EndDisabled();
		float controlsWidth = ImGui::GetItemRectSize().x;
		
		bool b_disableAxisStateControl;
		bool b_axisEnabled;
		std::string stateString;
		glm::vec4 stateColor;
		if(!b_axisConnected) {
			stateString = "No Axis";
			stateColor = Colors::darkRed;
			b_disableAxisStateControl = true;
			b_axisEnabled = false;
		}else{
			switch(mapping->getAxis()->getState()){
				case DeviceState::OFFLINE:
					stateString = "Offline";
					stateColor = Colors::blue;
					b_disableAxisStateControl = true;
					b_axisEnabled = false;
					break;
				case DeviceState::NOT_READY:
					stateString = "Not Ready";
					stateColor = Colors::red;
					b_disableAxisStateControl = true;
					b_axisEnabled = false;
					break;
				case DeviceState::READY:
					stateString = "Ready";
					stateColor = Colors::yellow;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::DISABLING:
					stateString = "Disabling...";
					stateColor = Colors::darkYellow;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::ENABLING:
					stateString = "Enabling...";
					stateColor = Colors::darkGreen;
					b_disableAxisStateControl = false;
					b_axisEnabled = false;
					break;
				case DeviceState::ENABLED:
					stateString = "Enabled";
					stateColor = Colors::green;
					b_disableAxisStateControl = false;
					b_axisEnabled = true;
					break;
			}
		}
		
		ImVec2 buttonSize(controlsWidth / 2.0, ImGui::GetTextLineHeight() * 2.0);
		ImGui::PushFont(Fonts::sansBold15);
		ImGui::BeginDisabled(b_disableAxisStateControl);
		if(b_axisEnabled){
			if(customButton("Disable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				mapping->disableAxis();
			}
		}else{
			if(customButton("Enable", buttonSize, ImGui::GetStyle().Colors[ImGuiCol_Button], ImGui::GetStyle().FrameRounding, ImDrawFlags_RoundCornersLeft)){
				mapping->enableAxis();
			}
		}
		ImGui::EndDisabled();
		ImGui::SameLine(0.0, 0.0);
		backgroundText(stateString.c_str(), buttonSize, stateColor, Colors::black, ImDrawFlags_RoundCornersRight);
		ImGui::PopFont();
		
		
		glm::vec2 nameBoxSize(controlsWidth, ImGui::GetTextLineHeightWithSpacing());
		if(b_axisConnected){
			std::string axisName = mapping->axisPin->getConnectedPin()->getSharedPointer<AxisInterface>()->getName();
			backgroundText(axisName.c_str(), nameBoxSize, Colors::darkGray);
		}
		else{
			backgroundText("No Axis", nameBoxSize, Colors::darkRed, Colors::red);
		}
		
		ImGui::EndGroup();
		ImGui::PopID();
	
		if(i < axisMappings.size() - 1) ImGui::SameLine();
	}
	
	
	
	
	
	ImGui::SameLine();
	
	float masterControlHeight = ImGui::GetItemRectSize().y - ImGui::GetTextLineHeightWithSpacing() - ImGui::GetStyle().ItemSpacing.y;
	
	ImGui::BeginGroup();
	static double min = -1.0;
	static double max = 1.0;
	ImVec2 masterSliderSize(ImGui::GetTextLineHeight() * 3.0, masterControlHeight);
	if(ImGui::VSliderScalar("##ManualVelocity", masterSliderSize, ImGuiDataType_Double, &masterVelocityCommand, &min, &max, "")){
		setMasterVelocityTarget(masterVelocityCommand);
	}
	if(ImGui::IsItemDeactivatedAfterEdit()){
		masterVelocityCommand = 0.0;
		setMasterVelocityTarget(0.0);
	}
	backgroundText("Master", ImVec2(masterSliderSize.x, ImGui::GetTextLineHeightWithSpacing()), Colors::darkGray, Colors::white);
	ImGui::EndGroup();
	
	ImGui::Checkbox("Enable Group Surveillance", &b_enableGroupSurveillance);
	
}
