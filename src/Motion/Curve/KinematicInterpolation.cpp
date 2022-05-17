#include <pch.h>

#include "Curve.h"

#define square(x) (x) * (x)

namespace Motion{

std::shared_ptr<TrapezoidalInterpolation> invalidInterpolation(std::shared_ptr<ControlPoint> startPoint, std::shared_ptr<ControlPoint> endPoint){
	std::shared_ptr<TrapezoidalInterpolation> output = std::make_shared<TrapezoidalInterpolation>();
	output->inPoint = startPoint;
	output->outPoint = endPoint;
	output->b_valid = false;
	output->validationError = ValidationError::INTERPOLATION_UNDEFINED;
	return output;
}

//get all interpolations with a given velocity
//end point time is not known here
bool getAllVelocityConstrainedInterpolations(std::shared_ptr<ControlPoint> startPoint,
											 std::shared_ptr<ControlPoint> endPoint,
											 double velocity,
											 TrapezoidalInterpolation output[16]) {

	
	if (startPoint->outAcceleration == 0.0 || endPoint->inAcceleration == 0.0 || velocity == 0.0) return false;
	const double pi = startPoint->position;
	const double ti = startPoint->time;
	const double vi = startPoint->velocity;
	const double po = endPoint->position;
	const double vo = endPoint->velocity;
	const double dp = endPoint->position - startPoint->position;

	//get solutions with coast phase
	for(uint8_t i = 0; i < 8; i++){

		TrapezoidalInterpolation& solution = output[i];

		double ai = startPoint->outAcceleration;
		double ao = endPoint->inAcceleration;
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
		solution.startTime = startPoint->time;			//time of curve start
		solution.startPosition = startPoint->position;	//position of curve start
		solution.startVelocity = startPoint->velocity;	//velocity at curve start
		solution.startAcceleration = ai;			//acceleration of curve
		solution.coastStartPosition = pi + dpi;		//position of curve after acceleration phase
		solution.coastStartTime = ti + dti;	        //time of acceleration end
		solution.coastVelocity = vt;			    //velocity of constant velocity phase
		solution.coastEndPosition = po - dpo;       //position of deceleration start
		solution.coastEndTime = ti + dti + dtt;     //time of deceleration start
		solution.endAcceleration = ao;			    //deceleration of curve
		solution.endTime = ti + dt;					//time of curve end
		solution.endPosition = endPoint->position;		//position of curve end
		solution.endVelocity = endPoint->velocity;		//velocity of curve end
		solution.inPoint = startPoint;
		solution.outPoint = endPoint;
		solution.validationError = ValidationError::NONE;
	}

	//get solutions without coast phase
	for(uint8_t i = 0; i < 8; i++){

		TrapezoidalInterpolation& solution = output[i+8];

		double ai = startPoint->outAcceleration;
		double ao = endPoint->inAcceleration;
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
		solution.startTime = startPoint->time;			//time of curve start
		solution.startPosition = startPoint->position;	//position of curve start
		solution.startVelocity = startPoint->velocity;	//velocity at curve start
		solution.startAcceleration = ai;	        //acceleration of curve

		solution.coastStartPosition = pi + dpi;		//position of curve after acceleration phase
		solution.coastEndPosition = pi + dpi;       //position of deceleration start
		solution.coastStartTime = ti + dti;		    //time of acceleration end
		solution.coastEndTime = ti + dti;		    //time of deceleration start
		solution.coastVelocity = vt;			    //velocity of constant velocity phase

		solution.endAcceleration = ao;	            //deceleration of curve
		solution.endTime = ti + dti + dto;	        //time of curve end
		solution.endPosition = endPoint->position;		//position of curve end
		solution.endVelocity = endPoint->velocity;		//velocity of curve end
		
		solution.inPoint = startPoint;
		solution.outPoint = endPoint;
		solution.validationError = ValidationError::NONE;
	}

	for(int i = 0; i < 16; i++) if(output[i].b_valid) return true;
	return false;
}












//returns a profile using the position, velocity, acceleration and time data of the two specified points, while respecting the provided motion constraints

//get a profile matching the exact distance and time between the given motion points
//returns success or failure of profile creation if mathematically impossible due to the given constraints

std::shared_ptr<TrapezoidalInterpolation> TrapezoidalInterpolation::getTimeConstrained(std::shared_ptr<ControlPoint>& startPoint,
																					   std::shared_ptr<ControlPoint>& endPoint){
	
	//if one of these is zero, no profile can be generated
	if (startPoint->outAcceleration == 0.0 || endPoint->inAcceleration == 0.0) return nullptr;

	const double pi = startPoint->position;
	const double ti = startPoint->time;
	const double vi = startPoint->velocity;

	const double po = endPoint->position;
	const double to = endPoint->time;
	const double vo = endPoint->velocity;

	const double dt = endPoint->time - startPoint->time;
	const double dp = endPoint->position - startPoint->position;

	TrapezoidalInterpolation solutions[8];
	bool b_foundValidSolution = false;

	for(uint8_t i = 0; i < 8; i++){

		TrapezoidalInterpolation& solution = solutions[i];
		
		//try every comibation of signs for acceleration values and square root content
		double ai = startPoint->outAcceleration;
		double ao = endPoint->inAcceleration;
		if(i & 0x1) ai *= -1.0;
		if(i & 0x2) ao *= -1.0;
		bool rootTermSign = i & 0x4;

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
		solution.startTime = startPoint->time;	        //time of curve start
		solution.startPosition = startPoint->position;  //position of curve start
		solution.startVelocity = startPoint->velocity;  //velocity at curve start
		solution.startAcceleration = ai;	        	//acceleration of curve
		solution.coastStartPosition = pi + dpi;     	//position of curve after acceleration phase
		solution.coastStartTime = ti + dti;         	//time of acceleration end
		solution.coastVelocity = vt;			    	//velocity of constant velocity phase
		solution.coastEndPosition = po - dpo;       	//position of deceleration start
		solution.coastEndTime = to - dto;		    	//time of deceleration start
		solution.endAcceleration = ao;	            	//deceleration of curve
		solution.endTime = to;			            	//time of curve end
		solution.endPosition = endPoint->position;		//position of curve end
		solution.endVelocity = endPoint->velocity;		//velocity of curve end
		solution.inPoint = startPoint;
		solution.outPoint = endPoint;
		solution.b_valid = true;
		solution.validationError = ValidationError::NONE;

	}

	//return false if no solution exists for the specified parameters
	if (!b_foundValidSolution) return invalidInterpolation(startPoint, endPoint);
	
	//if there is one or more solutions, get the fastest solutions that is slower than the target time
	//this should not happen, but we need to pick a solution in case there is more than one
	TrapezoidalInterpolation* solution = nullptr;
	double fastestTime = INFINITY;
	for(int i = 0; i < 8; i++){
		if(solutions[i].b_valid && solutions[i].endTime < fastestTime){
			solution = &solutions[i];
			fastestTime = solutions[i].endTime;
		}
	}
	
	return std::make_shared<TrapezoidalInterpolation>(*solution);
}








//return the fastest profile using the position, velocity and acceleration values of the two specified points, while respecting those motion constraints
std::shared_ptr<TrapezoidalInterpolation> TrapezoidalInterpolation::getVelocityConstrained(std::shared_ptr<ControlPoint>& startPoint,
																						   std::shared_ptr<ControlPoint>& endPoint,
																						   double velocity){
	TrapezoidalInterpolation solutions[16];
	if(!getAllVelocityConstrainedInterpolations(startPoint, endPoint, velocity, solutions)) return invalidInterpolation(startPoint, endPoint);

	TrapezoidalInterpolation* fastestSolution = nullptr;
	double fastestTime = INFINITY;
	for(int i = 0; i < 16; i++){
		TrapezoidalInterpolation& solution = solutions[i];
		if(solution.b_valid && solution.endTime < fastestTime){
			fastestTime = solution.endTime;
			fastestSolution = &solution;
		}
	}
	
	return std::make_shared<TrapezoidalInterpolation>(*fastestSolution);
}








//get the best matching time based profile, not faster than the specified time
std::shared_ptr<TrapezoidalInterpolation> TrapezoidalInterpolation::getTimeConstrainedOrSlower(std::shared_ptr<ControlPoint>& startPoint,
																							   std::shared_ptr<ControlPoint>& endPoint,
																							   double maxVelocity){
	
	//first we try finding a solution that matches the requested time exactly and respects the max velocity
	std::shared_ptr<TrapezoidalInterpolation> timeConstrainedSolution = TrapezoidalInterpolation::getTimeConstrained(startPoint, endPoint);
	if(timeConstrainedSolution && timeConstrainedSolution->coastVelocity <= maxVelocity) return timeConstrainedSolution;

	//if that solution does not exists we get all profiles that use the max velocity
	//if no solution is found here, there is no solution)
	TrapezoidalInterpolation solutions[16];
	if (!getAllVelocityConstrainedInterpolations(startPoint, endPoint, maxVelocity, solutions)) return invalidInterpolation(startPoint, endPoint);

	//we then pick the solution hat is closest to the requested time but still slower
	
	//TODO: it can happen that there are no solutions slower than the requested time, in this case select the slowest one
	double fastestTime = INFINITY;
	TrapezoidalInterpolation* fastestSolution = nullptr;
	double minEndTime = endPoint->time;
	for(int i = 0; i < 16; i++){
		TrapezoidalInterpolation& solution = solutions[i];
		if(solution.b_valid && solution.endTime >= minEndTime && solution.endTime < fastestTime){
			fastestTime = solution.endTime;
			fastestSolution = &solution;
		}
	}

	if(fastestSolution) return std::make_shared<TrapezoidalInterpolation>(*fastestSolution);
	return invalidInterpolation(startPoint, endPoint);
}











TrapezoidalInterpolation::Phase TrapezoidalInterpolation::getPhaseAtTime(double time){
	if(time <= startTime) return Phase::NOT_STARTED;
	else if(time <= coastStartTime) return Phase::RAMP_IN;
	else if(time <= coastEndTime) return Phase::COAST;
	else if(time <= endTime) return Phase::RAMP_OUT;
	else return Phase::FINISHED;
}

TrapezoidalInterpolation::Phase TrapezoidalInterpolation::getNextPhase(Phase phase){
	switch(phase){
		case Phase::NOT_STARTED: return Phase::RAMP_IN;
		case Phase::RAMP_IN: return Phase::COAST;
		case Phase::COAST: return Phase::RAMP_OUT;
		case Phase::RAMP_OUT: return Phase::FINISHED;
		case Phase::FINISHED: return Phase::FINISHED;
	}
}

void TrapezoidalInterpolation::getPointAtPhaseTime(double time, Phase phase, Point& output){}


TrapezoidalInterpolation::Solution TrapezoidalInterpolation::getTimeAtPosition(Phase phase, double position, double& time_a, double& time_b){
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

Point TrapezoidalInterpolation::getPointAtTime(double time){
	double deltaT;
	Point output{.time = time};
	switch(getPhaseAtTime(time)){
		case Phase::NOT_STARTED:
			output.position = startPosition;
			output.velocity = 0.0;
			output.acceleration = 0.0;
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
			output.velocity = 0.0;
			output.acceleration = 0.0;
			break;
	}
	return output;
}

double TrapezoidalInterpolation::getNextIncrementTime(double previousPulseTime, double incrementsPerUnit){
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


void TrapezoidalInterpolation::updateDisplayCurvePoints(){
	displayPoints.clear();
	displayPoints.reserve(32);
	double rampInResolution = (coastStartTime - startTime) / 15.0;
	for(int i = 0; i <= 15; i++) displayPoints.push_back(getPointAtTime(startTime + i * rampInResolution));
	double rampOutResolution = (endTime - coastEndTime) / 15.0;
	for(int i = 0; i <= 15; i++) displayPoints.push_back(getPointAtTime(coastEndTime + i * rampOutResolution));
	displayInflectionPoints.clear();
	displayInflectionPoints.push_back(getPointAtTime(coastStartTime));
	displayInflectionPoints.push_back(getPointAtTime(coastEndTime));
}


};
