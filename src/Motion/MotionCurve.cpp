#include <pch.h>
#include "MotionCurve.h"

namespace MotionCurve {

	CurveProfile getTimeConstrainedProfile(CurvePoint startPoint, CurvePoint endPoint, MotionConstraints constraints) {

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		double pi = startPoint.position;
		double ti = startPoint.time;
		double vi = startPoint.velocity;
		double ai = std::abs(startPoint.acceleration);

		double po = endPoint.position;
		double vo = endPoint.velocity;
		double ao = std::abs(endPoint.acceleration);

		double dt = endPoint.time - startPoint.time;
		double dp = endPoint.position - startPoint.position;
		double dv = endPoint.velocity - startPoint.velocity;

		bool ai_sign = true;
		bool ao_sign = false;
		bool rootTermSign = true;

		double aTerm;
		double bTerm;
		double cTerm;
		double rTerm;
		double vt;

		auto solveCurve = [&](MotionCurve::CurveProfile& profile) -> bool {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);

			//============ QUADRATIC FUNCTION FOR CONSTANT VELOCITY VALUE ============

			//Quadratic equation constants for ax² + bx + c = 0
			double a = ai - ao; //solution undefined if == 0
			double b = 2.0 * (ao * vi - ai * vo + ai * ao * dt);
			double c = ai * square(vo) - ao * square(vi) - 2.0 * ai * ao * dp;
			double r = square(b) - 4.0 * a * c; //quadratic root term
			aTerm = a;
			bTerm = b;
			cTerm = c;
			rTerm = r;

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

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / vt;
			double to = rampOutStartTime + dto;

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

			//checks for a successfull solve
			//if (a == 0.0) return false;
			if (r < 0.0 && a != 0.0) return false;
			if (dti < 0.0) return false;
			if (dto < 0.0) return false;
			if (dti > to - dto) return false;
			return true;
		};

		MotionCurve::CurveProfile profile;

		bool solved = false;
		for (int i = 0; i < 8; i++) {
			switch (i) {
			case 0: //000
				ai_sign = false;
				ao_sign = false;
				rootTermSign = false;
				break;
			case 1: //001
				ai_sign = false;
				ao_sign = false;
				rootTermSign = true;
				break;
			case 2: //010
				ai_sign = false;
				ao_sign = true;
				rootTermSign = false;
				break;
			case 3: //011
				ai_sign = false;
				ao_sign = true;
				rootTermSign = true;
				break;
			case 4: //100
				ai_sign = true;
				ao_sign = false;
				rootTermSign = false;
				break;
			case 5: //101
				ai_sign = true;
				ao_sign = false;
				rootTermSign = true;
				break;
			case 6: //110
				ai_sign = true;
				ao_sign = true;
				rootTermSign = false;
				break;
			case 7: //111
				ai_sign = true;
				ao_sign = true;
				rootTermSign = true;
				break;
			}
			solved = solveCurve(profile);
			if (solved) break;
		}

		double maxV = 10.0;
		if (!solved || std::abs(vt) > maxV) {
			std::vector<CurveProfile> velocityConstrainedSolutions = MotionCurve::getVelocityContrainedProfiles(startPoint, endPoint, maxV, constraints);
			//TODO: choose a suitable profile from the list
		}
		return profile;
	}

	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================

	
    std::vector<CurveProfile> getVelocityContrainedProfiles(const CurvePoint& startPoint, const CurvePoint& endPoint, double velocity, const MotionConstraints& constraints) {

		std::vector<CurveProfile> solutions;

		auto square = [](double in) -> double { return std::pow(in, 2.0); };

		double pi = startPoint.position;
		double ti = startPoint.time;
		double vi = startPoint.velocity;
		double ai = std::abs(startPoint.acceleration);

		double vt = velocity;

		double po = endPoint.position;
		double vo = endPoint.velocity;
		double ao = std::abs(endPoint.acceleration);

		double dp = endPoint.position - startPoint.position;
		double dv = endPoint.velocity - startPoint.velocity;

		bool ai_sign = false;
		bool ao_sign = false;
		bool velocity_sign = false;

		auto solveCurveForVelocity = [&](MotionCurve::CurveProfile& profile) -> bool {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);
			if (!velocity_sign) vt = -std::abs(vt);
			else vt = std::abs(vt);

			double a = (vt - vi) / ai;
			double b = (vo - vt) / ao;
			double c = (square(vt) - square(vi)) / (2.0 * ai);
			double d = (square(vo) - square(vt)) / (2.0 * ao);
			double dt = a + b + vt * (dp - c - d);

			//solve end

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dpi = vi * dti + ai * square(dti) / 2.0;
			double dpo = vt * dto + ao * square(dto) / 2.0;

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = po - dpo;
			double rampOutStartTime = rampInEndTime + (rampOutStartPosition - rampInEndPosition) / vt;
			double to = rampOutStartTime + dto;

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

			if (dti < 0.0) return false;
			if (dto < 0.0) return false;
			if (dti > to - dto) return false;
			return dt > 0.0;
		};

		for (int i = 0; i < 8; i++) {
			switch (i) {
			case 0: //000
				ai_sign = false;
				ao_sign = false;
				velocity_sign = false;
				break;
			case 1: //001
				ai_sign = false;
				ao_sign = false;
				velocity_sign = true;
				break;
			case 2: //010
				ai_sign = false;
				ao_sign = true;
				velocity_sign = false;
				break;
			case 3: //011
				ai_sign = false;
				ao_sign = true;
				velocity_sign = true;
				break;
			case 4: //100
				ai_sign = true;
				ao_sign = false;
				velocity_sign = false;
				break;
			case 5: //101
				ai_sign = true;
				ao_sign = false;
				velocity_sign = true;
				break;
			case 6: //110
				ai_sign = true;
				ao_sign = true;
				velocity_sign = false;
				break;
			case 7: //111
				ai_sign = true;
				ao_sign = true;
				velocity_sign = true;
				break;
			}
			MotionCurve::CurveProfile profile;
			if (solveCurveForVelocity(profile)) solutions.push_back(profile);
		}


		bool squareRootSign = false;

		auto solveTangentCurve = [&](MotionCurve::CurveProfile& profile) {

			//initialize acceleration signs
			if (!ai_sign) ai = -std::abs(ai);
			else ai = std::abs(ai);
			if (!ao_sign) ao = -std::abs(ao);
			else ao = std::abs(ao);

			double dpi = (square(vo) - square(vi) - 2.0 * ao * dp) / (2.0 * (ai - ao));
			double r = square(vi) + 2.0 * ai * dpi;
			//choose square root solution sign
			if (!squareRootSign) vt = -std::sqrt(r);
			else vt = std::sqrt(r);

			double dti = (vt - vi) / ai;
			double dto = (vo - vt) / ao;
			double dpo = (square(vo) - square(vt)) / (2.0 * ao);

			double rampInEndPosition = pi + dpi;
			double rampInEndTime = ti + dti;
			double rampOutStartPosition = rampInEndPosition;
			double rampOutStartTime = rampInEndTime;
			double to = rampOutStartTime + dto;

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
			if (ai == ao) return false;
			if (r < 0) return false;
			if (dti < 0.0) return false;
			if (dto < 0.0) return false;
			if (dti + dto > to) return false;
			if (std::abs(vt) > velocity) return false;
			return true;
		};

		for (int i = 0; i < 8; i++) {
			switch (i) {
			case 0: //000
				ai_sign = false;
				ao_sign = false;
				squareRootSign = false;
				break;
			case 1: //001
				ai_sign = false;
				ao_sign = false;
				squareRootSign = true;
				break;
			case 2: //010
				ai_sign = false;
				ao_sign = true;
				squareRootSign = false;
				break;
			case 3: //011
				ai_sign = false;
				ao_sign = true;
				squareRootSign = true;
				break;
			case 4: //100
				ai_sign = true;
				ao_sign = false;
				squareRootSign = false;
				break;
			case 5: //101
				ai_sign = true;
				ao_sign = false;
				squareRootSign = true;
				break;
			case 6: //110
				ai_sign = true;
				ao_sign = true;
				squareRootSign = false;
				break;
			case 7: //111
				ai_sign = true;
				ao_sign = true;
				squareRootSign = true;
				break;
			}
			MotionCurve::CurveProfile profile;
			if (solveTangentCurve(profile)) solutions.push_back(profile);
		}
		return solutions;
    }
	
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================
	//=======================================================================================================================================





	bool isInsideCurveTime(double time, CurveProfile& curvePoints) {
		return false;
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





