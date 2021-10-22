#pragma once

namespace Timing {

	double getProgramTime_seconds();
	double getProgramTime_milliseconds();
	double getProgramTime_microseconds();
	long long getProgramTime_nanoseconds();

	double getSystemTime_seconds();
	double getSystemTime_milliseconds();
	double getSystemTime_microseconds();
	long long getSystemTime_nanoseconds();
};