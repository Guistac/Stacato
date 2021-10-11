#include <pch.h>
#include "MotionCurve.h"

namespace MotionCurve {

	bool getTimeConstrainedProfile(const CurvePoint& startPoint, const CurvePoint& endPoint, double maxVelocity, CurveProfile& output) {

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		const double pi = startPoint.position;
		const double ti = startPoint.time;
		const double vi = startPoint.velocity;
		double ai = std::abs(startPoint.acceleration);

		const double po = endPoint.position;
		const double to = endPoint.time;
		const double vo = endPoint.velocity;
		double ao = std::abs(endPoint.acceleration);

		//if one of these is zero, no profile can be generated
		if (ai == 0.0 || ao == 0.0 || maxVelocity == 0.0) return false;

		const double dt = endPoint.time - startPoint.time;
		const double dp = endPoint.position - startPoint.position;
		const double dv = endPoint.velocity - startPoint.velocity;

		//solution choice variables
		bool ai_sign = true;
		bool ao_sign = false;
		bool rootTermSign = true;

		//output
		double vt;

		auto solveCurve = [&]() -> CurveProfile {

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
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = to - dto;

			CurveProfile profile;
			profile.rampInStartTime = startPoint.time;			//time of curve start
			profile.rampInStartPosition = startPoint.position;	//position of curve start
			profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
			profile.rampInAcceleration = ai;	//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.coastVelocity = vt;			//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.rampOutAcceleration = ao;	//deceleration of curve
			profile.rampOutEndTime = to;			    //time of curve end
			profile.rampOutEndPosition = endPoint.position;		//position of curve end
			profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end
			//TODO: add a continuity check to see if the coast phase actually arrives at the rampout begin

			//reject unreal and illogical solutions
			if (r < 0.0 && a != 0.0) profile.isDefined = false;
			else if (dti < 0.0) profile.isDefined = false;
			else if (dto < 0.0) profile.isDefined = false;
			else if (dti > to - dto) profile.isDefined = false;
			else if (std::abs(vt) > std::abs(maxVelocity)) profile.isDefined = false;
			else profile.isDefined = true;

			return profile;
		};

		std::vector<CurveProfile> timeConstrainedSolutions;

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			rootTermSign = (c >> 2) & 0x1;
			CurveProfile profile = solveCurve();
			if (profile.isDefined) timeConstrainedSolutions.push_back(profile);
		}

		if (timeConstrainedSolutions.empty()) {
			//if no solution exists for a time constrained profile
			//we try finding profiles based on the max velocity
			std::vector<CurveProfile> velocityConstrainedSolutions;
			//if no solution is found here, there is no solution
			if (!MotionCurve::getVelocityContrainedProfiles(startPoint, endPoint, maxVelocity, velocityConstrainedSolutions)) return false;

			//first filter out all profiles that are slower or equal to the requested time
			std::vector<CurveProfile> slowerThanRequestedSolutions;
			for (auto& solution : velocityConstrainedSolutions) {
				if (solution.rampOutEndTime >= endPoint.time) slowerThanRequestedSolutions.push_back(solution);
			}

			if (slowerThanRequestedSolutions.empty()) {
				//if there is no profile slower than the requested time we pick the slowest profile
				CurveProfile* slowestProfileBelowRequested = &velocityConstrainedSolutions.front();
				for (int i = 1; i < velocityConstrainedSolutions.size(); i++) {
					if (velocityConstrainedSolutions[i].rampOutEndTime > slowestProfileBelowRequested->rampOutEndTime)
						slowestProfileBelowRequested = &velocityConstrainedSolutions[i];
				}
				output = *slowestProfileBelowRequested;
				return true;
			}
			else {
				//else we pick the fastest profile of the remaining ones
				CurveProfile* fastestProfileAboveRequested = &slowerThanRequestedSolutions.front();
				for (int i = 0; i < slowerThanRequestedSolutions.size(); i++) {
					if (slowerThanRequestedSolutions[i].rampOutEndTime < fastestProfileAboveRequested->rampOutEndTime)
						fastestProfileAboveRequested = &slowerThanRequestedSolutions[i];
				}
				output = *fastestProfileAboveRequested;
				return true;
			}
		}
		else {
			CurveProfile* fastestProfileAboveRequested = &timeConstrainedSolutions.front();
			for (auto& solution : timeConstrainedSolutions) {
				if (solution.rampOutEndTime < fastestProfileAboveRequested->rampOutEndTime)
					fastestProfileAboveRequested = &solution;
			}
			output = *fastestProfileAboveRequested;
			return true;
		}
	}

	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================

	
    bool getVelocityContrainedProfiles(const CurvePoint& startPoint, const CurvePoint& endPoint, double velocity, std::vector<CurveProfile>& output) {

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		const double pi = startPoint.position;
		const double ti = startPoint.time;
		const double vi = startPoint.velocity;
		double ai = std::abs(startPoint.acceleration);

		double vt = velocity;

		const double po = endPoint.position;
		const double vo = endPoint.velocity;
		double ao = std::abs(endPoint.acceleration);

		if (ai == 0.0 || ao == 0.0 || vt == 0.0) return false;

		const double dp = endPoint.position - startPoint.position;
		const double dv = endPoint.velocity - startPoint.velocity;

		bool ai_sign = false;
		bool ao_sign = false;
		bool velocity_sign = false;

		auto solveCurveForVelocity = [&]() -> CurveProfile {

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
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / vt;
			double to = rampOutStartTime + dto;

			CurveProfile profile;
			profile.rampInStartTime = startPoint.time;			//time of curve start
			profile.rampInStartPosition = startPoint.position;	//position of curve start
			profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
			profile.rampInAcceleration = ai;					//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.coastVelocity = vt;							//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.rampOutAcceleration = ao;					//deceleration of curve
			profile.rampOutEndTime = to;						//time of curve end
			profile.rampOutEndPosition = endPoint.position;		//position of curve end
			profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end

			//reject illogical solutions
			if (dti < 0.0) profile.isDefined = false;
			else if (dto < 0.0) profile.isDefined = false;
			else if (dti > to - dto) profile.isDefined = false;
			else profile.isDefined = dt > 0.0;

			return profile;
		};

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			velocity_sign = (c >> 2) & 0x1;
			CurveProfile profile = solveCurveForVelocity();
			if (profile.isDefined) output.push_back(profile);
		}


		bool squareRootSign = false;

		auto solveTangentCurve = [&]() -> CurveProfile {

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

			CurveProfile profile;
			profile.rampInStartTime = startPoint.time;			//time of curve start
			profile.rampInStartPosition = startPoint.position;	//position of curve start
			profile.rampInStartVelocity = startPoint.velocity;	//velocity at curve start
			profile.rampInAcceleration = ai;	//acceleration of curve
			profile.rampInEndPosition = rampInEndPosition;		//position of curve after acceleration phase
			profile.rampInEndTime = rampInEndTime;			    //time of acceleration end
			profile.coastVelocity = vt;			//velocity of constant velocity phase
			profile.rampOutStartPosition = rampOutStartPosition;//position of deceleration start
			profile.rampOutStartTime = rampOutStartTime;		//time of deceleration start
			profile.rampOutAcceleration = ao;	//deceleration of curve
			profile.rampOutEndTime = to;			    //time of curve end
			profile.rampOutEndPosition = endPoint.position;		//position of curve end
			profile.rampOutEndVelocity = endPoint.velocity;		//velocity of curve end

			//reject non-real and illogical solutions
			if (ai == ao) profile.isDefined = false;
			else if (r < 0) profile.isDefined = false;
			else if (dti < 0.0) profile.isDefined = false;
			else if (dto < 0.0) profile.isDefined = false;
			else if (dti + dto > to) profile.isDefined = false;
			else if (std::abs(vt) > velocity) profile.isDefined = false;
			else profile.isDefined = true;

			return profile;
		};

		for (char c = 0; c < 8; c++) {
			ai_sign = c & 0x1;
			ao_sign = (c >> 1) & 0x1;
			squareRootSign = (c >> 2) & 0x1;
			MotionCurve::CurveProfile profile = solveTangentCurve();
			if (profile.isDefined) output.push_back(profile);
		}
		
		return !output.empty();
    }
	
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================


	bool getFastestVelocityConstrainedProfile(const CurvePoint& startPoint, const CurvePoint& endPoint, double velocity, CurveProfile& output) {
		std::vector<CurveProfile> velocityBasedProfiles;
		if (getVelocityContrainedProfiles(startPoint, endPoint, velocity, velocityBasedProfiles)) {
			CurveProfile* fastestProfile = &velocityBasedProfiles.front();
			for (int i = 1; i < velocityBasedProfiles.size(); i++) {
				if (velocityBasedProfiles[i].rampOutEndTime < fastestProfile->rampOutEndTime)
					fastestProfile = &velocityBasedProfiles[i];
			}
			output = *fastestProfile;
			return true;
		}
		return false;
	}

	bool isInsideCurveTime(double time, CurveProfile& profile) {
		return time >= profile.rampInStartTime && time <= profile.rampOutEndTime;
	}

	CurvePoint getCurvePointAtTime(double time, CurveProfile& curveProfile) {
        CurvePoint output;
        output.time = time;
        double deltaT;
        switch (getCurvePhaseAtTime(time, curveProfile)) {
            case CurvePhase::NOT_STARTED:
                output.position = curveProfile.rampInStartPosition;
                output.velocity = curveProfile.rampInStartVelocity;
                output.acceleration = 0.0;
                break;
            case CurvePhase::RAMP_IN:
                deltaT = time - curveProfile.rampInStartTime;
                output.position = curveProfile.rampInStartPosition + curveProfile.rampInStartVelocity * deltaT + curveProfile.rampInAcceleration * std::pow(deltaT, 2.0) / 2.0;
                output.velocity = curveProfile.rampInStartVelocity + curveProfile.rampInAcceleration * deltaT;
                output.acceleration = curveProfile.rampInAcceleration;
                break;
            case CurvePhase::COAST:
                deltaT = time - curveProfile.rampInEndTime;
                output.position = curveProfile.rampInEndPosition + deltaT * curveProfile.coastVelocity;
                output.velocity = curveProfile.coastVelocity;
                output.acceleration = 0.0;
                break;
            case CurvePhase::RAMP_OUT:
                deltaT = time - curveProfile.rampOutStartTime;
                output.position = curveProfile.rampOutStartPosition + curveProfile.coastVelocity * deltaT + curveProfile.rampOutAcceleration * std::pow(deltaT, 2.0) / 2.0;
                output.velocity = curveProfile.coastVelocity + curveProfile.rampOutAcceleration * deltaT;
                output.acceleration = curveProfile.rampOutAcceleration;
                break;
            case CurvePhase::FINISHED:
                output.position = curveProfile.rampOutEndPosition;
                output.velocity = curveProfile.rampOutEndVelocity;
                output.acceleration = 0.0;
                break;
        }
		return output;
	}

	CurvePhase getCurvePhaseAtTime(double time, CurveProfile& curveProfile) {
        if (time < curveProfile.rampInStartTime) return CurvePhase::NOT_STARTED;
        else if (time < curveProfile.rampInEndTime) return CurvePhase::RAMP_IN;
        else if (time < curveProfile.rampOutStartTime) return CurvePhase::COAST;
        else if (time < curveProfile.rampOutEndTime) return CurvePhase::RAMP_OUT;
        else return CurvePhase::FINISHED;
	}

}





