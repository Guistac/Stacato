#include <pch.h>

void cCurvesTest();

namespace MotionTest{

	struct Point{
		double position;
		double velocity;
		double time;
		double acceleration;
		int phase;
	};

	class Interpolation{
	public:
		
		enum class Phase{
			NOT_STARTED,
			RAMP_IN,
			COAST,
			RAMP_OUT,
			FINISHED
		};
		
		double startTime;
		double startPosition;
		double startVelocity;
		double startAcceleration;

		double coastStartTime;
		double coastStartPosition;
		double coastVelocity;
		double coastEndTime;
		double coastEndPosition;

		double endTime;
		double endPosition;
		double endVelocity;
		double endAcceleration;

		bool b_valid = false;

		bool containsTime(double time);
		Phase getPhaseAtTime(double time);
		void getPointAtTime(double time, Point& point);
		double getNextIncrementTime(double previousPulseTime, double incrementsPerUnit);
		
	private:
		Phase getNextPhase(Phase phase);
		void getPointAtPhaseTime(double time, Phase phase, Point& output);
		enum class Solution{
			NONE,
			SINGLE,
			DOUBLE
		};
		Solution getTimeAtPosition(Phase phase, double position, double& time_a, double& time_b);
		
	};


Point matchPosition(Point previous, Point target, double deltaT, double maxVelocity, double acceleration);


	bool getTimedOrSlowerInterpolation(const Point& start, const Point& end, double maxVelocity, Interpolation& output);
	bool getFastestVelocityConstrainedInterpolation(const Point& start, const Point& end, double velocity, Interpolation& output);

};
