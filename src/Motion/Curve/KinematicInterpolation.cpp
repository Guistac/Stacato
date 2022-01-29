#include <pch.h>

#include "Curve.h"

namespace Motion::TrapezoidalInterpolation {


	bool getTimeConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, std::shared_ptr<Interpolation>& output) {

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double vi = startPoint->velocity;
		double ai = std::abs(startPoint->acceleration);

		const double po = endPoint->position;
		const double to = endPoint->time;
		const double vo = endPoint->velocity;
		double ao = std::abs(endPoint->acceleration);

		//if one of these is zero, no profile can be generated
		if (ai == 0.0 || ao == 0.0) {
			output->inPoint = startPoint;
			output->outPoint = endPoint;
			output->b_valid = false;
			if (ai == 0.0) output->validationError = ValidationError::INTERPOLATION_INPUT_ACCELERATION_IS_ZERO;
			else if (ao == 0.0) output->validationError = ValidationError::INTERPOLATION_OUTPUT_ACCELERATION_IS_ZERO;
			return false;
		}

		const double dt = endPoint->time - startPoint->time;
		const double dp = endPoint->position - startPoint->position;
		const double dv = endPoint->velocity - startPoint->velocity;

		//solution choice variables
		bool ai_sign = true;
		bool ao_sign = false;
		bool rootTermSign = true;

		//output
		double vt;

		auto solveCurve = [&]() -> Interpolation {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);

			//============ QUADRATIC FUNCTION FOR CONSTANT VELOCITY VALUE ============

			//Quadratic equation constants for ax² + bx + c = 0
			double a = ai - ao;
			double b = 2.0 * (ao * vi - ai * vo + ai * ao * dt);
			double c = ai * square(vo) - ao * square(vi) - 2.0 * ai * ao * dp;
			double r = square(b) - 4.0 * a * c; //quadratic root term

			if (a == 0) {
				//if the a term is zero, the problems turns into a simple linear function
				vt = -c / b;
			}
			else {
				//quadratic solutions
				double vt_plus = (-b + sqrt(r)) / (2.0 * a);
				double vt_minus = (-b - sqrt(r)) / (2.0 * a);
				if (rootTermSign) vt = vt_plus;
				else vt = vt_minus;
			}

			//=========================================================================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dtt = dt - dti - dto;
			double dpt = dtt * vt;
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = to - dto;

			Interpolation profile;
			profile.inTime = startPoint->time;			//time of curve start
			profile.inPosition = startPoint->position;	//position of curve start
			profile.inVelocity = startPoint->velocity;	//velocity at curve start
			profile.inAcceleration = ai;	//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.interpolationVelocity = vt;			//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.outAcceleration = ao;	//deceleration of curve
			profile.outTime = to;			    //time of curve end
			profile.outPosition = endPoint->position;		//position of curve end
			profile.outVelocity = endPoint->velocity;		//velocity of curve end

			//reject unreal and illogical solutions
			if (r < 0.0 && a != 0.0) profile.b_valid = false;
			else if (dtt < 0.0 || dti < 0.0 || dto < 0.0) profile.b_valid = false;
			else profile.b_valid = true;

			return profile;
		};

		std::vector<Interpolation> timeConstrainedSolutions;

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			rootTermSign = (c >> 2) & 0x1;
			Interpolation profile = solveCurve();
			if (profile.b_valid) timeConstrainedSolutions.push_back(profile);
		}

		//return false if no solution exists for the specified parameters
		if (timeConstrainedSolutions.empty()) {
			output->inPoint = startPoint;
			output->outPoint = endPoint;
			output->b_valid = false;
			output->validationError = ValidationError::INTERPOLATION_UNDEFINED;
			return false;
		}
		
		//if there is one or more solutions, get the fastest solutions that is slower than the target time
		//this should not happen, but we need to pick a solution in case there is more than one
		Interpolation* fastestProfileAboveRequested = &timeConstrainedSolutions.front();
		for (auto& solution : timeConstrainedSolutions) {
			if (solution.outTime < fastestProfileAboveRequested->outTime)
				fastestProfileAboveRequested = &solution;
		}
		Interpolation solution = *fastestProfileAboveRequested;	

		output->inTime = solution.inTime;
		output->inPosition = solution.inPosition;
		output->inVelocity = solution.inVelocity;
		output->inAcceleration = solution.inAcceleration;
		output->outTime = solution.outTime;
		output->outPosition = solution.outPosition;
		output->outVelocity = solution.outVelocity;
		output->outAcceleration = solution.outAcceleration;
		output->rampInEndTime = solution.rampInEndTime;
		output->rampInEndPosition = solution.rampInEndPosition;
		output->rampOutStartTime = solution.rampOutStartTime;
		output->rampOutStartPosition = solution.rampOutStartPosition;
		output->interpolationVelocity = solution.interpolationVelocity;
		output->b_valid = true;
		output->inPoint = startPoint;
		output->outPoint = endPoint;
		startPoint->time = solution.inTime;
		startPoint->position = solution.inPosition;
		startPoint->velocity = solution.inVelocity;
		startPoint->acceleration = 0.0;
		startPoint->rampOut = solution.inAcceleration;
		startPoint->velocityOut = solution.inVelocity;
		startPoint->outInterpolation = output;
		endPoint->time = solution.outTime;
		endPoint->position = solution.outPosition;
		endPoint->velocity = solution.outVelocity;
		endPoint->acceleration = 0.0;
		endPoint->rampIn = solution.outAcceleration;
		endPoint->velocityIn = solution.outVelocity;
		endPoint->inInterpolation = output;
		output->type = InterpolationType::TRAPEZOIDAL;
		return true;
	}



	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================


	bool getVelocityContrainedInterpolations(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::vector<Interpolation>& output) {

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		const double pi = startPoint->position;
		const double ti = startPoint->time;
		const double vi = startPoint->velocity;
		double ai = std::abs(startPoint->acceleration);

		double vt = velocity;

		const double po = endPoint->position;
		const double vo = endPoint->velocity;
		double ao = std::abs(endPoint->acceleration);

		if (ai == 0.0 || ao == 0.0 || vt == 0.0) return false;

		const double dp = endPoint->position - startPoint->position;
		const double dv = endPoint->velocity - startPoint->velocity;

		bool ai_sign = false;
		bool ao_sign = false;
		bool velocity_sign = false;

		auto solveCurveForVelocity = [&]() -> Interpolation {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);
			if (!velocity_sign) vt = -std::abs(vt);
			else vt = std::abs(vt);

			//====================== FUNCTION FOR VELOCITY BASED SOLVE =========================

			double a = (vt - vi) / ai;
			double b = (vo - vt) / ao;
			double c = (square(vt) - square(vi)) / (2.0 * ai);
			double d = (square(vo) - square(vt)) / (2.0 * ao);
			double dt = a + b + vt * (dp - c - d);

			//==================================================================================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dtt = dt - dti - dto;
			double dpt = dtt * vt;
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / vt;
			double to = rampOutStartTime + dto;

			Interpolation profile;
			profile.inTime = startPoint->time;			//time of curve start
			profile.inPosition = startPoint->position;	//position of curve start
			profile.inVelocity = startPoint->velocity;	//velocity at curve start
			profile.inAcceleration = ai;					//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.interpolationVelocity = vt;							//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.outAcceleration = ao;					//deceleration of curve
			profile.outTime = to;						//time of curve end
			profile.outPosition = endPoint->position;		//position of curve end
			profile.outVelocity = endPoint->velocity;		//velocity of curve end

			//reject illogical solutions
			if (dt < 0.0 || dtt < 0.0 || dto < 0.0 || dti < 0.0) profile.b_valid = false;
			else profile.b_valid = true;

			return profile;
		};

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			velocity_sign = (c >> 2) & 0x1;
			Interpolation profile = solveCurveForVelocity();
			if (profile.b_valid) output.push_back(profile);
		}


		bool squareRootSign = false;

		auto solveTangentCurve = [&]() -> Interpolation {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);

			//==================== FUNCTION FOR SOLVING WITHOUT COAST PHASE ======================

			double dpi = (square(vo) - square(vi) - 2.0 * ao * dp) / (2.0 * (ai - ao));
			double r = square(vi) + 2.0 * ai * dpi;
			//choose square root solution sign
			if (!squareRootSign) vt = -std::sqrt(r);
			else vt = std::sqrt(r);

			//====================================================================================

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dpo = (square(vo) - square(vt)) / (2.0 * ao);

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = rampInEndPosition;
			double rampOutStartTime = rampInEndTime;
			double to = rampOutStartTime + dto;

			Interpolation profile;
			profile.inTime = startPoint->time;			//time of curve start
			profile.inPosition = startPoint->position;	//position of curve start
			profile.inVelocity = startPoint->velocity;	//velocity at curve start
			profile.inAcceleration = ai;	//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.interpolationVelocity = vt;			//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.outAcceleration = ao;	//deceleration of curve
			profile.outTime = to;			    //time of curve end
			profile.outPosition = endPoint->position;		//position of curve end
			profile.outVelocity = endPoint->velocity;		//velocity of curve end

			//reject non-real and illogical solutions
			if (ai == ao) profile.b_valid = false;
			else if (r < 0) profile.b_valid = false;
			else if (dti < 0.0) profile.b_valid = false;
			else if (dto < 0.0) profile.b_valid = false;
			else if (dti + dto > to) profile.b_valid = false;
			else if (std::abs(vt) > velocity) profile.b_valid = false;
			else profile.b_valid = true;

			return profile;
		};

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			squareRootSign = (c >> 2) & 0x1;
			Interpolation profile = solveTangentCurve();
			if (profile.b_valid) output.push_back(profile);
		}

		return !output.empty();
	}

	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================



	bool getFastestVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double velocity, std::shared_ptr<Interpolation>& output) {
		std::vector<Interpolation> velocityBasedProfiles;
		if (getVelocityContrainedInterpolations(startPoint, endPoint, velocity, velocityBasedProfiles)) {
			Interpolation* solution = &velocityBasedProfiles.front();
			for (int i = 1; i < velocityBasedProfiles.size(); i++) {
				if (velocityBasedProfiles[i].outTime < solution->outTime)
					solution = &velocityBasedProfiles[i];
			}
			output->inTime = solution->inTime;
			output->inPosition = solution->inPosition;
			output->inVelocity = solution->inVelocity;
			output->inAcceleration = solution->inAcceleration;
			output->outTime = solution->outTime;
			output->outPosition = solution->outPosition;
			output->outVelocity = solution->outVelocity;
			output->outAcceleration = solution->outAcceleration;
			output->rampInEndTime = solution->rampInEndTime;
			output->rampInEndPosition = solution->rampInEndPosition;
			output->rampOutStartTime = solution->rampOutStartTime;
			output->rampOutStartPosition = solution->rampOutStartPosition;
			output->interpolationVelocity = solution->interpolationVelocity;
			output->b_valid = true;
			output->inPoint = startPoint;
			output->outPoint = endPoint;
			startPoint->time = solution->inTime;
			startPoint->position = solution->inPosition;
			startPoint->velocity = solution->inVelocity;
			startPoint->acceleration = 0.0;
			startPoint->rampOut = solution->inAcceleration;
			startPoint->velocityOut = solution->inVelocity;
			startPoint->outInterpolation = output;
			endPoint->time = solution->outTime;
			endPoint->position = solution->outPosition;
			endPoint->velocity = solution->outVelocity;
			endPoint->acceleration = 0.0;
			endPoint->rampIn = solution->outAcceleration;
			endPoint->velocityIn = solution->outVelocity;
			endPoint->inInterpolation = output;
			output->type = InterpolationType::TRAPEZOIDAL;
			return true;
		}
		return false;
	}



	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================


	bool getClosestTimeAndVelocityConstrainedInterpolation(std::shared_ptr<ControlPoint>& startPoint, std::shared_ptr<ControlPoint>& endPoint, double maxVelocity, std::shared_ptr<Interpolation>& output) {
		
		//first we try finding a solution that matches the requested time exactly and respects the max velocity
		if (getTimeConstrainedInterpolation(startPoint, endPoint, output) && std::abs(output->interpolationVelocity) <= maxVelocity) return true;

		//if that solution does not exists we get all profiles that use the max velocity
		//we then pick the solution hat is closest to the requested time but still slower

		Interpolation solution;

		//if no solution exists for a time constrained profile
		//we try finding profiles based on the max velocity
		std::vector<Interpolation> velocityConstrainedSolutions;
		//if no solution is found here, there is no solution)

		if (!getVelocityContrainedInterpolations(startPoint, endPoint, maxVelocity, velocityConstrainedSolutions)) return false;

		//first filter out all profiles that are slower or equal to the requested time
		std::vector<Interpolation*> slowerThanRequestedSolutions;
		for (auto& solution : velocityConstrainedSolutions) {
			if (solution.outTime >= endPoint->time) slowerThanRequestedSolutions.push_back(&solution);
		}

		if (slowerThanRequestedSolutions.empty()) {
			//if there is no profile slower than the requested time we pick the slowest profile
			Interpolation* slowestProfileBelowRequested = &velocityConstrainedSolutions.front();
			for (int i = 1; i < velocityConstrainedSolutions.size(); i++) {
				if (velocityConstrainedSolutions[i].outTime > slowestProfileBelowRequested->outTime)
					slowestProfileBelowRequested = &velocityConstrainedSolutions[i];
			}
			solution = *slowestProfileBelowRequested;
		}
		else {
			//else we pick the fastest profile of the remaining ones
			Interpolation* fastestProfileAboveRequested = slowerThanRequestedSolutions.front();
			for (int i = 0; i < slowerThanRequestedSolutions.size(); i++) {
				if (slowerThanRequestedSolutions[i]->outTime < fastestProfileAboveRequested->outTime)
					fastestProfileAboveRequested = slowerThanRequestedSolutions[i];
			}
			solution = *fastestProfileAboveRequested;
		}

		output->inTime = solution.inTime;
		output->inPosition = solution.inPosition;
		output->inVelocity = solution.inVelocity;
		output->inAcceleration = solution.inAcceleration;
		output->outTime = solution.outTime;
		output->outPosition = solution.outPosition;
		output->outVelocity = solution.outVelocity;
		output->outAcceleration = solution.outAcceleration;
		output->rampInEndTime = solution.rampInEndTime;
		output->rampInEndPosition = solution.rampInEndPosition;
		output->rampOutStartTime = solution.rampOutStartTime;
		output->rampOutStartPosition = solution.rampOutStartPosition;
		output->interpolationVelocity = solution.interpolationVelocity;
		output->b_valid = true;
		output->inPoint = startPoint;
		output->outPoint = endPoint;
		startPoint->time = solution.inTime;
		startPoint->position = solution.inPosition;
		startPoint->velocity = solution.inVelocity;
		startPoint->acceleration = 0.0;
		startPoint->rampOut = solution.inAcceleration;
		startPoint->velocityOut = solution.inVelocity;
		startPoint->outInterpolation = output;
		endPoint->time = solution.outTime;
		endPoint->position = solution.outPosition;
		endPoint->velocity = solution.outVelocity;
		endPoint->acceleration = 0.0;
		endPoint->rampIn = solution.outAcceleration;
		endPoint->velocityIn = solution.outVelocity;
		endPoint->inInterpolation = output;
		output->type = InterpolationType::TRAPEZOIDAL;
		return true;
	}

}





