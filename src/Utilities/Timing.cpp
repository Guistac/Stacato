#include <pch.h>

#include "timing.h"

namespace Timing {

	std::chrono::system_clock::time_point programStart = std::chrono::system_clock::now();

	double getProgramTime_seconds() {
		return (double)getProgramTime_nanoseconds() / 1000000000.0L;
	}

	long long getProgramTime_nanoseconds() {
		std::chrono::duration delay = std::chrono::system_clock::now() - programStart;
		std::chrono::nanoseconds delayNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
		return delayNanoseconds.count();
	}

	double getSystemTime_seconds() {
		return (double)getSystemTime_nanoseconds() / 1000000000.0;
	}

	double getSystemTime_milliseconds() {
		return (double)getSystemTime_nanoseconds() / 1000000.0;
	}

	long long getSystemTime_nanoseconds() {
		using namespace std::chrono;
		return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count();
	}

}