#include <pch.h>

#include "timing.h"

namespace Timing {

	using namespace std::chrono;

	system_clock::time_point programStartTime = system_clock::now();

	double getProgramTime_seconds() {
		return (double)getProgramTime_nanoseconds() / 1000000000.0;
	}

	double getProgramTime_milliseconds() {
		return (double)getProgramTime_nanoseconds() / 1000000.0;
	}

	double getProgramTime_microseconds() {
		return (double)getProgramTime_nanoseconds() / 1000.0;
	}

	long long getProgramTime_nanoseconds() {
		return duration_cast<nanoseconds>(system_clock::now() - programStartTime).count();
	}

	double getSystemTime_seconds() {
		return (double)getSystemTime_nanoseconds() / 1000000000.0;
	}

	double getSystemTime_milliseconds() {
		return (double)getSystemTime_nanoseconds() / 1000000.0;
	}

	double getSystemTime_microseconds() {
		return (double)getSystemTime_nanoseconds() / 1000.0;
	}

	long long getSystemTime_nanoseconds() {
		return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
	}

}