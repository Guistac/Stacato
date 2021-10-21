#pragma once

#include <chrono>

namespace Timing {

	double getProgramTime_seconds();
	long long getProgramTime_nanoseconds();

	double getSystemTime_seconds();
	long long getSystemTime_nanoseconds();
};