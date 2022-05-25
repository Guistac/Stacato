#include "C_Curves.h"

#include <imgui.h>
#include <implot.h>

#include "Motion/Curve/Curve.h"
#include "Motion/Curve/Profile.h"

void cCurvesTest(){
	
	static double startTime = 0;
	static double startPosition = 0;
	static double startVelocity = 0;
	static double startAcceleration = 1;
	
	static double endPosition = 1;
	static double endVelocity = 0;
	static double endAcceleration = 1;
	static double endTime = 1;
	
	static double targetVelocity = 1;
	static double maxVelocity = 1;
	static bool b_timeConstraint = false;
	
	static bool b_showPulses = false;
	static int pulsesPerUnit = 50;
	
	static int pointsPerSecond = 100;
	
	static bool b_catchUp = true;
	static bool b_constrainCatchupStartToCurve = false;
	static double catchUpTime = 0.0;
	static double catchUpPosition = -0.1;
	static double catchupVelocity = 0.0;
	static double maxCatchupVelocity = 2.0;
	static double maxCatchupAcceleration = 2.0;
	
	static double upperPositionLimit = 10.0;
	static double lowerPositionLimit = -10.0;
	static bool b_enablePositionLimits = false;
	
	//——————————————————————————————
	//		 Settings Panels
	//——————————————————————————————
	
	ImGui::BeginChild("test", ImVec2(350, 430), true);
	float inputFieldWidth = 150.0;
	
	ImGui::SetNextItemWidth(inputFieldWidth * 2);
	if(ImGui::BeginCombo("##Type", b_timeConstraint ? "Time Constrained" : "Velocity Constrained")){
		if(ImGui::Selectable("Time Constrained", b_timeConstraint)) b_timeConstraint = true;
		if(ImGui::Selectable("Velocity Constrained", !b_timeConstraint)) b_timeConstraint = false;
		ImGui::EndCombo();
	}
	
	ImGui::Separator();
	
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Time", &startTime, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Position", &startPosition, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Velocity", &startVelocity, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Acceleration", &startAcceleration, 0.1, 1.0);
	
	ImGui::Separator();
	
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("End Position", &endPosition, 0.1, 1.0);
	ImGui::BeginDisabled(!b_timeConstraint);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("End Time", &endTime, 0.1, 1.0);
	ImGui::EndDisabled();
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("End Velocity", &endVelocity, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("End Acceleration", &endAcceleration, 0.1, 1.0);
	
	ImGui::Separator();
	
	if(b_timeConstraint) {
		ImGui::SetNextItemWidth(inputFieldWidth);
		ImGui::InputDouble("Max Velocity", &maxVelocity, 0.1, 1.0);
		double targetTime = endTime - startTime;
		ImGui::SetNextItemWidth(inputFieldWidth);
		if(ImGui::InputDouble("Target Time", &targetTime, 0.1, 1.0)){
			endTime = startTime + targetTime;
		}
	}
	else {
		ImGui::SetNextItemWidth(inputFieldWidth);
		ImGui::InputDouble("Target Velocity", &targetVelocity, 0.1, 1.0);
	}
	
	ImGui::Separator();
	
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputInt("Points Per Second", &pointsPerSecond);
	
	ImGui::EndChild();
	
	ImGui::SameLine();
	
	ImGui::BeginChild("options", ImVec2(350, 430), true);
		
	ImGui::Checkbox("Show Pulses", &b_showPulses);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputInt("Pulses Per Unit", &pulsesPerUnit);
	
	ImGui::Checkbox("Show Catchup", &b_catchUp);
	ImGui::Checkbox("Constrain Catchup Start to Curve", &b_constrainCatchupStartToCurve);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Time", &catchUpTime, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Position", &catchUpPosition, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Start Velocity", &catchupVelocity, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Max Velocity", &maxCatchupVelocity, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Max Acceleration", &maxCatchupAcceleration, 0.1, 1.0);
	
	ImGui::Checkbox("Enable Position Limits", &b_enablePositionLimits);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Min Position", &lowerPositionLimit, 0.1, 1.0);
	ImGui::SetNextItemWidth(inputFieldWidth);
	ImGui::InputDouble("Max Position", &upperPositionLimit, 0.1, 1.0);
	
	ImGui::EndChild();
	
	
	//——————————————————————————————
	//		 	Processing
	//——————————————————————————————
	
	
	if(b_enablePositionLimits){
		startPosition = std::min(startPosition, upperPositionLimit);
		startPosition = std::max(startPosition, lowerPositionLimit);
		endPosition = std::min(endPosition, upperPositionLimit);
		endPosition = std::max(endPosition, lowerPositionLimit);
		catchUpPosition = std::min(catchUpPosition, upperPositionLimit);
		catchUpPosition = std::max(catchUpPosition, lowerPositionLimit);
	}
	
	if(pointsPerSecond < 1) pointsPerSecond = 1;
	if(maxCatchupVelocity < 0.1) maxCatchupVelocity = 0.1;
	if(maxCatchupAcceleration < 0.1) maxCatchupAcceleration = 0.1;
	if(targetVelocity < 0.0) targetVelocity = 0.0;
	if(startAcceleration < 0.0) startAcceleration = 0.0;
	if(endAcceleration < 0.0) endAcceleration = 0.0;
	
	
	auto startPoint = std::make_shared<Motion::ControlPoint>();
	startPoint->position = startPosition;
	startPoint->velocity = startVelocity;
	startPoint->outAcceleration = startAcceleration;
	startPoint->time = startTime;
	
	auto endPoint = std::make_shared<Motion::ControlPoint>();
	endPoint->position = endPosition;
	endPoint->velocity = endVelocity;
	endPoint->inAcceleration = endAcceleration;
	endPoint->time = endTime;
	
	std::shared_ptr<Motion::TrapezoidalInterpolation> interpolation;
	if(b_timeConstraint) interpolation = Motion::TrapezoidalInterpolation::getTimeConstrainedOrSlower(startPoint, endPoint, maxVelocity);
	else interpolation = Motion::TrapezoidalInterpolation::getVelocityConstrained(startPoint, endPoint, targetVelocity);
	bool b_solutionFound = interpolation != nullptr;
	
	if(b_constrainCatchupStartToCurve) catchUpPosition = interpolation->getPointAtTime(catchUpTime).position;
	
	//——————————————————————————————
	//		 	Info Panel
	//——————————————————————————————
	
	ImGui::SameLine();
	
	ImGui::BeginChild("Info", ImVec2(ImGui::GetContentRegionAvail().x, 350), true);
	
	if(!b_solutionFound) ImGui::Text("No Solution Found");
	ImGui::BeginDisabled(!b_solutionFound);
	ImGui::Text("Movement Time: %.6fs", interpolation->endTime - interpolation->startTime);
	ImGui::Text("Movement Coast Velocity: %.6f u/s", interpolation->coastVelocity);
	ImGui::Separator();
	ImGui::Text("End Position: %.6f", interpolation->endPosition);
	ImGui::Text("End Time: %.6f", interpolation->endTime);
	ImGui::Text("End Velocity: %.6f", interpolation->endVelocity);
	ImGui::Text("End Acceleration: %.6f", interpolation->endAcceleration);
	ImGui::EndDisabled();
	
	ImGui::EndChild();

	
	
	//——————————————————————————————
	//	  Graphs Point Generation
	//——————————————————————————————
	
	std::vector<double> time;
	std::vector<double> position;
	std::vector<double> velocity;
	std::vector<double> acceleration;
	int pointCount = 0;
	
	std::vector<glm::vec2> pulsations;
	pulsations.reserve(100000);
	
	std::vector<double> catchupPositionPoints;
	std::vector<double> catchupVelocityPoints;
	std::vector<double> catchupAccelerationPoints;
	std::vector<double> catchupTimePoints;
	std::vector<double> catchupPhasePoints;
	
	double catchupTime = 0.0;
	double catchupPosition = 0.0;
	bool b_caughtUp = false;
	
	if(b_solutionFound){
		
		int expectedPointCount = (interpolation->endTime - interpolation->startTime) * pointsPerSecond;
		if(expectedPointCount > 1000000) expectedPointCount = 1000000;
		time.reserve(expectedPointCount * 2);
		position.reserve(expectedPointCount * 2);
		velocity.reserve(expectedPointCount * 2);
		acceleration.reserve(expectedPointCount * 2);
		
		double t = interpolation->startTime;
		while(t < interpolation->endTime){
			Motion::Point p = interpolation->getPointAtTime(t);
			time.push_back(t);
			position.push_back(p.position);
			velocity.push_back(p.velocity);
			acceleration.push_back(p.acceleration);
			t += (1.0 / pointsPerSecond);
			pointCount++;
			if(pointCount > expectedPointCount) break;
		}
		t = interpolation->endTime;
		Motion::Point p = interpolation->getPointAtTime(t);
		time.push_back(t);
		position.push_back(p.position);
		velocity.push_back(p.velocity);
		acceleration.push_back(p.acceleration);
		pointCount++;
		

		if(b_showPulses){
			double pulseTime = interpolation->startTime;
			pulsations.push_back(glm::vec2(interpolation->startTime, 1.0));
			while(pulseTime < interpolation->endTime){
				double previousPulseTime = pulseTime;
				pulseTime = interpolation->getNextIncrementTime(pulseTime, pulsesPerUnit);
				double fallingEdgeTime = previousPulseTime + (pulseTime - previousPulseTime) / 2.0;
				if(pulseTime == DBL_MAX) break;
				pulsations.push_back(glm::vec2(fallingEdgeTime, 1.0));
				pulsations.push_back(glm::vec2(fallingEdgeTime, 0.0));
				pulsations.push_back(glm::vec2(pulseTime, 0.0));
				pulsations.push_back(glm::vec2(pulseTime, 1.0));
			}
			pulsations.push_back(glm::vec2(interpolation->endTime, 1.0));
		}
		
		if(b_catchUp){
			
			double deltaT = 1.0 / pointsPerSecond;
			double t = catchUpTime;
			
			Motion::Profile profile;
			profile.setPosition(catchUpPosition);
			profile.setVelocity(catchupVelocity);
			profile.setAcceleration(0.0);
			
			catchupTimePoints.push_back(t);
			catchupPositionPoints.push_back(catchupPosition);
			catchupVelocityPoints.push_back(catchupVelocity);
			catchupAccelerationPoints.push_back(0.0);
			//catchupPhasePoints.push_back(0);
			
			while(true){
				t += deltaT;
				Motion::Point target = interpolation->getPointAtTime(t);
				if(b_enablePositionLimits) b_caughtUp = profile.matchPositionAndRespectPositionLimits(deltaT,
																									  target.position,
																									  target.velocity,
																									  target.acceleration,
																									  maxCatchupAcceleration,
																									  maxCatchupVelocity,
																									  lowerPositionLimit,
																									  upperPositionLimit);
				else b_caughtUp = profile.matchPosition(deltaT,
														target.position,
														target.velocity,
														target.acceleration,
														maxCatchupAcceleration,
														maxCatchupVelocity);
				
				catchupTimePoints.push_back(t);
				catchupPositionPoints.push_back(profile.getPosition());
				catchupVelocityPoints.push_back(profile.getVelocity());
				catchupAccelerationPoints.push_back(profile.getAcceleration());
				//catchupPhasePoints.push_back(output.phase);
				
				if(b_caughtUp){
					catchupTime = t;
					catchupPosition = profile.getPosition();
					break;
				}
				else if(t > 100.0) break;
			}
			
		}
	}
	
	//——————————————————————————————
	//	  		Graphs Display
	//——————————————————————————————
	
	
	if(ImPlot::BeginPlot("##Test", nullptr, nullptr, ImGui::GetContentRegionAvail())){
		if(b_solutionFound){
			ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 1.0);
			ImPlot::PlotLine("Acceleration", time.data(), acceleration.data(), pointCount);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Velocity", time.data(), velocity.data(), pointCount);
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 4.0);
			ImPlot::PlotLine("Position", time.data(), position.data(), pointCount);
			
			if(b_showPulses){
				ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0);
				ImPlot::PlotLine("Pulsations", &pulsations.front().x, &pulsations.front().y, pulsations.size(), 0, sizeof(glm::vec2));
			}
			
			if(b_catchUp){
				ImPlot::SetNextLineStyle(ImVec4(.0f, 1.f, .0f, 1.f), 1.0);
				ImPlot::PlotLine("CatchupPosition", &catchupTimePoints.front(), &catchupPositionPoints.front(), catchupPositionPoints.size(), 0, sizeof(double));
				ImPlot::PlotLine("CatchupVelocity", &catchupTimePoints.front(), &catchupVelocityPoints.front(), catchupVelocityPoints.size(), 0, sizeof(double));
				ImPlot::PlotLine("CatchupAcceleration", &catchupTimePoints.front(), &catchupAccelerationPoints.front(), catchupAccelerationPoints.size(), 0, sizeof(double));
				ImPlot::PlotLine("CatchupPhase", &catchupTimePoints.front(), &catchupPhasePoints.front(), catchupPhasePoints.size(), 0, sizeof(double));
				ImPlot::DragPoint("##CatchUpStart", &catchUpTime, &catchUpPosition);
				if(b_caughtUp){
					ImPlot::PlotScatter("CatchupPosition", &catchupTime, &catchupPosition, 1);
				}
			}
			
			
			if(b_timeConstraint){
				double targetTimeLine[2] = {time.back(), endTime};
				double targetPositionLine[2] = {position.back(), endPosition};
				ImPlot::PlotLine("##LineToTarget", targetTimeLine, targetPositionLine, 2);
				ImPlot::PlotScatter("End", &targetTimeLine[0], &targetPositionLine[0], 1);
				
				ImPlot::DragPoint("End", &endTime, &endPosition);
				ImPlot::DragPoint("Start", &startTime, &startPosition);
			}else{
				
				ImPlot::DragPoint("Start", &startTime, &startPosition);
				ImPlot::DragLineY("End", &endPosition);
				ImPlot::SetNextMarkerStyle(ImPlotMarker_Circle, 5.0, ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 0);
				ImPlot::PlotScatter("End", &time.back(), &position.back(), 1);
			}
				
			
			
		}
		ImPlot::EndPlot();
	}
	
}
