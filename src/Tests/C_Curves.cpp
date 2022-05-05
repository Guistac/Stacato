#include "C_Curves.h"

#include <imgui.h>
#include <implot.h>

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
	
	static int pulsesPerUnit = 50;
	
	MotionTest::Interpolation solution;
	MotionTest::Point start = {
		.position = startPosition,
		.velocity = startVelocity,
		.time = startTime,
		.acceleration = startAcceleration
	};
	MotionTest::Point end = {
		.position = endPosition,
		.velocity = endVelocity,
		.time = endTime,
		.acceleration = endAcceleration
	};
	
	ImGui::BeginChild("test", ImVec2(350, 410), true);
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
	
	ImGui::InputInt("Pulses Per Unit", &pulsesPerUnit);
	
	ImGui::EndChild();
	
	if(targetVelocity < 0.0) targetVelocity = 0.0;
	if(startAcceleration < 0.0) startAcceleration = 0.0;
	if(endAcceleration < 0.0) endAcceleration = 0.0;
	
	bool b_solutionFound;
	if(b_timeConstraint) b_solutionFound = MotionTest::getTimedOrSlowerInterpolation(start, end, maxVelocity, solution);
	else b_solutionFound = MotionTest::getFastestVelocityConstrainedInterpolation(start, end, targetVelocity, solution);
	
	
	
	ImGui::SameLine();
	
	ImGui::BeginChild("Info", ImVec2(ImGui::GetContentRegionAvail().x, 350), true);
	
	if(!b_solutionFound) ImGui::Text("No Solution Found");
	ImGui::BeginDisabled(!b_solutionFound);
	ImGui::Text("Movement Time: %.6fs", solution.endTime - solution.startTime);
	ImGui::Text("Movement Coast Velocity: %.6f u/s", solution.coastVelocity);
	ImGui::Separator();
	ImGui::Text("End Position: %.6f", solution.endPosition);
	ImGui::Text("End Time: %.6f", solution.endTime);
	ImGui::Text("End Velocity: %.6f", solution.endVelocity);
	ImGui::Text("End Acceleration: %.6f", solution.endAcceleration);
	ImGui::EndDisabled();
	
	ImGui::EndChild();

	std::vector<double> time;
	std::vector<double> position;
	std::vector<double> velocity;
	std::vector<double> acceleration;
	int pointCount = 0;
	
	std::vector<glm::vec2> pulsations;
	pulsations.reserve(100000);
	
	if(b_solutionFound){
		int pointsPerSecond = 100;
		int expectedPointCount = (solution.endTime - solution.startTime) * pointsPerSecond;
		if(expectedPointCount > 1000000) expectedPointCount = 1000000;
		time.reserve(expectedPointCount * 2);
		position.reserve(expectedPointCount * 2);
		velocity.reserve(expectedPointCount * 2);
		acceleration.reserve(expectedPointCount * 2);
		MotionTest::Point point;
		
		double t = solution.startTime;
		while(t < solution.endTime){
			solution.getPointAtTime(t, point);
			time.push_back(t);
			position.push_back(point.position);
			velocity.push_back(point.velocity);
			acceleration.push_back(point.acceleration);
			t += (1.0 / pointsPerSecond);
			pointCount++;
			if(pointCount > expectedPointCount) break;
		}
		t = solution.endTime;
		solution.getPointAtTime(t, point);
		time.push_back(t);
		position.push_back(point.position);
		velocity.push_back(point.velocity);
		acceleration.push_back(point.acceleration);
		pointCount++;
		
		/*
		int pulsationsPerUnit = 50;
		double pulseUnit = 1.0 / pulsationsPerUnit;
		double half = pulseUnit / 2.0;
		double tt = solution.startTime;
		double inc = 0.001;
		MotionTest::Point p;
		
		while(tt < solution.endTime){
			
			glm::vec2 pulsePoint;
			pulsePoint.x = tt;
			
			solution.getPointAtTime(tt, p);
			
			double remainder = fmod(p.position, pulseUnit);
			if(abs(remainder) < half) pulsePoint.y = 1.0;
			else pulsePoint.y = 0.0;
			
			pulsations.push_back(pulsePoint);
			
			
			tt += inc;
		}
		 */
		
		double pulseTime = solution.startTime;
		pulsations.push_back(glm::vec2(solution.startTime, 1.0));
		while(pulseTime < solution.endTime){
			double previousPulseTime = pulseTime;
			pulseTime = solution.getNextIncrementTime(pulseTime, pulsesPerUnit);
			double fallingEdgeTime = previousPulseTime + (pulseTime - previousPulseTime) / 2.0;
			if(pulseTime == DBL_MAX) break;
			pulsations.push_back(glm::vec2(fallingEdgeTime, 1.0));
			pulsations.push_back(glm::vec2(fallingEdgeTime, 0.0));
			pulsations.push_back(glm::vec2(pulseTime, 0.0));
			pulsations.push_back(glm::vec2(pulseTime, 1.0));
		}
		pulsations.push_back(glm::vec2(solution.endTime, 1.0));
	}
	
	
	if(ImPlot::BeginPlot("##Test", nullptr, nullptr, ImGui::GetContentRegionAvail())){
		if(b_solutionFound){
			ImPlot::SetNextLineStyle(ImVec4(0.5f, 0.5f, 0.5f, 1.0f), 1.0);
			ImPlot::PlotLine("Acceleration", time.data(), acceleration.data(), pointCount);
			ImPlot::SetNextLineStyle(ImVec4(0.0f, 0.0f, 1.0f, 1.0f), 2.0);
			ImPlot::PlotLine("Velocity", time.data(), velocity.data(), pointCount);
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), 4.0);
			ImPlot::PlotLine("Position", time.data(), position.data(), pointCount);
			
			ImPlot::SetNextLineStyle(ImVec4(1.0f, 1.0f, 1.0f, 1.0f), 1.0);
			ImPlot::PlotLine("Pulsations", &pulsations.front().x, &pulsations.front().y, pulsations.size(), 0, sizeof(glm::vec2));
			
			
			
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






















#define square(x) (x) * (x)

namespace MotionTest{

	//get a profile matching the exact distance and time between the given motion points
	//returns success or failure of profile creation if mathematically impossible due to the given constraints
	bool getTimeConstrainedInterpolation(const Point& start, const Point& end, Interpolation& output){

		//if one of these is zero, no profile can be generated
		if (start.acceleration == 0.0 || end.acceleration == 0.0) return false;

		const double pi = start.position;
		const double ti = start.time;
		const double vi = start.velocity;

		const double po = end.position;
		const double to = end.time;
		const double vo = end.velocity;

		const double dt = end.time - start.time;
		const double dp = end.position - start.position;

		Interpolation solutions[8];
		bool b_foundValidSolution = false;

		for(uint8_t i = 0; i < 8; i++){
			//try every comibation of signs for acceleration values and square root content
			bool ai_sign = i & 0x1;
			bool ao_sign = i & 0x2;
			bool rootTermSign = i & 0x4;

			Interpolation& solution = solutions[i];

			double ai = start.acceleration;
			double ao = end.acceleration;
			if(ai_sign) ai *= -1.0;
			if(ao_sign) ao *= -1.0;

			//============ QUADRATIC FUNCTION FOR CONSTANT VELOCITY VALUE ============

			//Quadratic equation constants for ax² + bx + c = 0
			double a = ai - ao;
			double b = 2.0 * (ao * vi - ai * vo + ai * ao * dt);
			double c = ai * square(vo) - ao * square(vi) - 2.0 * ai * ao * dp;
			double r = square(b) - 4.0 * a * c; //quadratic root term

			//if the content of the square root is negative, this solution in not valid
			if(r < 0.0 && a != 0.0){
				solution.b_valid = false;
				continue;
			}

			double vt; //coast velocity is unknown

			//if the a term is zero, the problems turns into a simple linear function
			if (a == 0.0) vt = -c / b;
			else {
				//quadratic solutions
				double vt_plus = (-b + sqrt(r)) / (2.0 * a);
				double vt_minus = (-b - sqrt(r)) / (2.0 * a);
				vt = rootTermSign ? vt_plus : vt_minus;
			}

			//=========================================================================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dtt = dt - dti - dto;

			//reject solutions with negative time delta
			if (dtt < 0.0 || dti < 0.0 || dto < 0.0){
				solution.b_valid = false;
				continue;
			}

			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			b_foundValidSolution = true;
			solution.b_valid = true;
			solution.startTime = start.time;	        //time of curve start
			solution.startPosition = start.position;    //position of curve start
			solution.startVelocity = start.velocity;    //velocity at curve start
			solution.startAcceleration = ai;	        //acceleration of curve
			solution.coastStartPosition = pi + dpi;     //position of curve after acceleration phase
			solution.coastStartTime = ti + dti;         //time of acceleration end
			solution.coastVelocity = vt;			    //velocity of constant velocity phase
			solution.coastEndPosition = po - dpo;       //position of deceleration start
			solution.coastEndTime = to - dto;		    //time of deceleration start
			solution.endAcceleration = ao;	            //deceleration of curve
			solution.endTime = to;			            //time of curve end
			solution.endPosition = end.position;	    //position of curve end
			solution.endVelocity = end.velocity;		    //velocity of curve end

		}

		//return false if no solution exists for the specified parameters
		if (!b_foundValidSolution) return false;
		
		//if there is one or more solutions, get the fastest solutions that is slower than the target time
		//this should not happen, but we need to pick a solution in case there is more than one
		Interpolation* solution = nullptr;
		double fastestTime = INFINITY;
		for(int i = 0; i < 8; i++){
			if(solutions[i].b_valid && solutions[i].endTime < fastestTime){
				solution = &solutions[i];
				fastestTime = solutions[i].endTime;
			}
		}
		output = *solution;
		return true;
	}







	//get all interpolations with a given velocity
	//end point time is not known here
	bool getVelocityConstrainedInterpolations(const Point& start, const Point& end, double velocity, Interpolation output[16]) {

		if (start.acceleration == 0.0 || end.acceleration == 0.0 || velocity == 0.0) return false;

		const double pi = start.position;
		const double ti = start.time;
		const double vi = start.velocity;
		
		const double po = end.position;
		const double vo = end.velocity;
		
		const double dp = end.position - start.position;

		//get solutions with coast phase
		for(uint8_t i = 0; i < 8; i++){

			Interpolation& solution = output[i];

			double ai = start.acceleration;
			double ao = end.acceleration;
			double vt = velocity;

			if(i & 0x1) ai *= -1.0;
			if(i & 0x2) ao *= -1.0;
			if(i & 0x4) vt *= -1.0;

			//====================== FUNCTION FOR VELOCITY BASED SOLVE =========================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			
			if (dto < 0.0 || dti < 0.0) {
				solution.b_valid = false;
				continue;
			}
			
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;
			double dpt = dp - dpi - dpo;
			
			double dtt = dpt / vt;
			double dt = dti + dtt + dto;
			
			if(dt < 0.0 || dtt < 0.0){
				solution.b_valid = false;
				continue;
			}

			//==================================================================================

			solution.b_valid = true;
			solution.startTime = start.time;			//time of curve start
			solution.startPosition = start.position;	//position of curve start
			solution.startVelocity = start.velocity;	//velocity at curve start
			solution.startAcceleration = ai;			//acceleration of curve
			solution.coastStartPosition = pi + dpi;		//position of curve after acceleration phase
			solution.coastStartTime = ti + dti;	        //time of acceleration end
			solution.coastVelocity = vt;			    //velocity of constant velocity phase
			solution.coastEndPosition = po - dpo;       //position of deceleration start
			solution.coastEndTime = ti + dti + dtt;     //time of deceleration start
			solution.endAcceleration = ao;			    //deceleration of curve
			solution.endTime = ti + dt;					//time of curve end
			solution.endPosition = end.position;		//position of curve end
			solution.endVelocity = end.velocity;		//velocity of curve end
		}

		//get solutions without coast phase
		for(uint8_t i = 0; i < 8; i++){

			Interpolation& solution = output[i+8];

			double ai = start.acceleration;
			double ao = end.acceleration;

			if(i & 0x1) ai *= -1.0;
			if(i & 0x2) ao *= -1.0;
			bool b_squareRootSign = i & 0x4;

			//==================== FUNCTION FOR SOLVING WITHOUT COAST PHASE ======================

			if(ai == ao){
				solution.b_valid = false;
				continue;
			}

			double dpi = (square(vo) - square(vi) - 2.0 * ao * dp) / (2.0 * (ai - ao));
			double r = square(vi) + 2.0 * ai * dpi;

			if(r < 0){
				solution.b_valid = false;
				continue;
			}

			double vt = sqrt(r);
			if(b_squareRootSign) vt *= -1.0;

			if (abs(vt) > velocity) {
				solution.b_valid = false;
				continue;
			}

			//====================================================================================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;


			if (dti < 0.0 || dto < 0.0) {
				solution.b_valid = false;
				continue;
			}

			solution.b_valid = true;
			solution.startTime = start.time;			//time of curve start
			solution.startPosition = start.position;	//position of curve start
			solution.startVelocity = start.velocity;	//velocity at curve start
			solution.startAcceleration = ai;	        //acceleration of curve

			solution.coastStartPosition = pi + dpi;		//position of curve after acceleration phase
			solution.coastEndPosition = pi + dpi;       //position of deceleration start
			solution.coastStartTime = ti + dti;		    //time of acceleration end
			solution.coastEndTime = ti + dti;		    //time of deceleration start
			solution.coastVelocity = vt;			    //velocity of constant velocity phase

			solution.endAcceleration = ao;	            //deceleration of curve
			solution.endTime = ti + dti + dto;	        //time of curve end
			solution.endPosition = end.position;		//position of curve end
			solution.endVelocity = end.velocity;		//velocity of curve end

		}

		for(int i = 0; i < 16; i++) if(output[i].b_valid) return true;
		return false;
	}



	bool getFastestVelocityConstrainedInterpolation(const Point& start, const Point& end, double velocity, Interpolation& output){

		Interpolation solutions[16];
		if(!getVelocityConstrainedInterpolations(start, end, velocity, solutions)) return false;

		Interpolation* fastestSolution = nullptr;
		double fastestTime = INFINITY;
		for(int i = 0; i < 16; i++){
			Interpolation& solution = solutions[i];
			if(solution.b_valid && solution.endTime < fastestTime){
				fastestTime = solution.endTime;
				fastestSolution = &solution;
			}
		}

		output = *fastestSolution;
		return true;
	}




	bool getTimedOrSlowerInterpolation(const Point& start, const Point& end, double maxVelocity, Interpolation& output){
		
		//first we try finding a solution that matches the requested time exactly and respects the max velocity
		bool b_timeConstrainedSolutionFound = getTimeConstrainedInterpolation(start, end, output);
		bool b_timeConstrainedSolutionVelocityIsBelowMaxVelocity = abs(output.coastVelocity) <= maxVelocity;
		if(b_timeConstrainedSolutionFound && b_timeConstrainedSolutionVelocityIsBelowMaxVelocity) return true;

		//if that solution does not exists we get all profiles that use the max velocity
		//if no solution is found here, there is no solution)
		Interpolation solutions[16];
		if (!getVelocityConstrainedInterpolations(start, end, maxVelocity, solutions)) return false;

		//we then pick the solution hat is closest to the requested time but still slower
		
		//TODO: it can happen that there are no solutions slower than the requested time, in this case select the slowest one
		double fastestTime = INFINITY;
		Interpolation* fastestSolution = nullptr;
		double minEndTime = end.time;
		for(int i = 0; i < 16; i++){
			Interpolation& solution = solutions[i];
			if(solution.b_valid && solution.endTime >= minEndTime && solution.endTime < fastestTime){
				fastestTime = solution.endTime;
				fastestSolution = &solution;
			}
		}

		if(fastestSolution){
			output = *fastestSolution;
			return true;
		}

		return false;
	}








	Interpolation::Phase Interpolation::getPhaseAtTime(double time){
		if(time <= startTime) return Phase::NOT_STARTED;
		else if(time <= coastStartTime) return Phase::RAMP_IN;
		else if(time <= coastEndTime) return Phase::COAST;
		else if(time <= endTime) return Phase::RAMP_OUT;
		else return Phase::FINISHED;
	}

	void Interpolation::getPointAtPhaseTime(double time, Phase phase, Point& output){
		double deltaT;
		switch(phase){
			case Phase::NOT_STARTED:
				output.position = startPosition;
				output.velocity = startVelocity;
				output.acceleration = startAcceleration;
				break;
			case Phase::RAMP_IN:
				deltaT = time - startTime;
				output.position = startPosition + startVelocity * deltaT + startAcceleration * square(deltaT) / 2.0;
				output.velocity = startVelocity + startAcceleration * deltaT;
				output.acceleration = startAcceleration;
				break;
			case Phase::COAST:
				deltaT = time - coastStartTime;
				output.position = coastStartPosition + deltaT * coastVelocity;
				output.velocity = coastVelocity;
				output.acceleration = 0.0;
				break;
			case Phase::RAMP_OUT:
				deltaT = time - coastEndTime;
				output.position = coastEndPosition + coastVelocity * deltaT + endAcceleration * square(deltaT) / 2.0;
				output.velocity = coastVelocity + endAcceleration * deltaT;
				output.acceleration = endAcceleration;
				break;
			case Phase::FINISHED:
				output.position = endPosition;
				output.velocity = endVelocity;
				output.acceleration = endAcceleration;
				break;
		}
		output.time = time;
	}

	void Interpolation::getPointAtTime(double time, Point& output){
		getPointAtPhaseTime(time, getPhaseAtTime(time), output);
	}

	

	double Interpolation::getNextIncrementTime(double previousPulseTime, double incrementsPerUnit){
		
		Phase phase = getPhaseAtTime(previousPulseTime);
		Point previousPoint;
		getPointAtPhaseTime(previousPulseTime, phase, previousPoint);
		
		double previousPulseIndex_r = round(previousPoint.position * incrementsPerUnit);
		
		double previousIncrementPosition_Units = previousPulseIndex_r / incrementsPerUnit;
		double incrementPositionDelta_Units = 1.0 / incrementsPerUnit;
		double nextPulsePosition_a = previousIncrementPosition_Units + incrementPositionDelta_Units * 0.500001;
		double nextPulsePosition_b = previousIncrementPosition_Units - incrementPositionDelta_Units * 0.500001;
		
		double time_aa, time_ab, time_ba, time_bb;
		Solution solution_a = getTimeAtPosition(phase, nextPulsePosition_a, time_aa, time_ab);
		Solution solution_b = getTimeAtPosition(phase, nextPulsePosition_b, time_ba, time_bb);
		
		while(phase != Phase::FINISHED){
		
			double nextPulseTime = DBL_MAX;
			
			if(solution_a == Solution::SINGLE && time_aa < nextPulseTime && time_aa > previousPulseTime) nextPulseTime = time_aa;
			else if(solution_a == Solution::DOUBLE){
				if(time_aa < nextPulseTime && time_aa > previousPulseTime) nextPulseTime = time_aa;
				if(time_ab < nextPulseTime && time_ab > previousPulseTime) nextPulseTime = time_ab;
			}
			
			if(solution_b == Solution::SINGLE && time_ba < nextPulseTime && time_ba > previousPulseTime) nextPulseTime = time_ba;
			else if(solution_b == Solution::DOUBLE){
				if(time_ba < nextPulseTime && time_ba > previousPulseTime) nextPulseTime = time_ba;
				if(time_bb < nextPulseTime && time_bb > previousPulseTime) nextPulseTime = time_bb;
			}
			
			if(nextPulseTime < DBL_MAX) return nextPulseTime;
			
			phase = getNextPhase(phase);
			solution_a = getTimeAtPosition(phase, nextPulsePosition_a, time_aa, time_ab);
			solution_b = getTimeAtPosition(phase, nextPulsePosition_b, time_ba, time_bb);
			
		}
		
		return DBL_MAX;
		
	}


	Interpolation::Phase Interpolation::getNextPhase(Phase phase){
		switch(phase){
			case Phase::NOT_STARTED: return Phase::RAMP_IN;
			case Phase::RAMP_IN: return Phase::COAST;
			case Phase::COAST: return Phase::RAMP_OUT;
			case Phase::RAMP_OUT: return Phase::FINISHED;
			case Phase::FINISHED: return Phase::FINISHED;
		}
	}



	Interpolation::Solution Interpolation::getTimeAtPosition(Phase phase, double position, double& time_a, double& time_b){
		double rootTerm;
		switch(phase){
				
			case Phase::NOT_STARTED:
				
				if(position == startPosition) return Solution::SINGLE;
				return Solution::NONE;
		
			case Phase::RAMP_IN:
								
				rootTerm = square(startVelocity) - 2.0 * startAcceleration * startPosition + 2.0 * startAcceleration * position;
				
				if(rootTerm < 0.0) return Solution::NONE;
				else if(rootTerm == 0.0) {
					time_a = -startVelocity / startAcceleration;
					if(getPhaseAtTime(time_a) == phase) return Solution::SINGLE;
					else return Solution::NONE;
				}
				else{
					time_a = startTime + (-startVelocity + sqrt(rootTerm)) / startAcceleration;
					time_b = startTime + (-startVelocity - sqrt(rootTerm)) / startAcceleration;
					bool time_a_ok = getPhaseAtTime(time_a) == phase;
					bool time_b_ok = getPhaseAtTime(time_b) == phase;
					if(time_a_ok && time_b_ok) return Solution::DOUBLE;
					else if(time_a_ok && !time_b_ok) {
						time_b = time_a;
						return Solution::SINGLE;
					}
					else if(time_b_ok && !time_a_ok){
						time_a = time_b;
						return Solution::SINGLE;
					}else return Solution::NONE;
				}

			case Phase::COAST:
				
				time_a = coastStartTime + (position - coastStartPosition) / coastVelocity;
				if(getPhaseAtTime(time_a) == phase) return Solution::SINGLE;
				else return Solution::NONE;
				
			case Phase::RAMP_OUT:
								
				rootTerm = square(coastVelocity) - 2.0 * endAcceleration * coastEndPosition + 2.0 * endAcceleration * position;
				
				if(rootTerm < 0.0) return Solution::NONE;
				else if(rootTerm == 0.0) {
					time_a = -coastVelocity / endAcceleration;
					if(getPhaseAtTime(time_a) == phase) return Solution::SINGLE;
					else return Solution::NONE;
				}
				else{
					time_a = coastEndTime + (-coastVelocity + sqrt(rootTerm)) / endAcceleration;
					time_b = coastEndTime + (-coastVelocity - sqrt(rootTerm)) / endAcceleration;
					bool time_a_ok = getPhaseAtTime(time_a) == phase;
					bool time_b_ok = getPhaseAtTime(time_b) == phase;
					if(time_a_ok && time_b_ok) return Solution::DOUBLE;
					else if(time_a_ok && !time_b_ok) {
						time_b = time_a;
						return Solution::SINGLE;
					}
					else if(time_b_ok && !time_a_ok){
						time_a = time_b;
						return Solution::SINGLE;
					}else return Solution::NONE;
				}
				
			case Phase::FINISHED:
				
				if(position == endPosition) return Solution::SINGLE;
				return Solution::NONE;
				
		}
	}




}




