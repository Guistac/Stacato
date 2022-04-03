#pragma once

namespace Timing {

	using namespace std::chrono;

	inline long long getProgramTime_nanoseconds() {
		static system_clock::time_point programStartTime = system_clock::now();
		return duration_cast<nanoseconds>(system_clock::now() - programStartTime).count();
	}

	inline double getProgramTime_microseconds() { return (double)getProgramTime_nanoseconds() / 1000.0; }

	inline double getProgramTime_milliseconds() { return (double)getProgramTime_nanoseconds() / 1000000.0; }

	inline double getProgramTime_seconds() { return (double)getProgramTime_nanoseconds() / 1000000000.0; }

	inline void start(){ getProgramTime_nanoseconds(); }


	inline long long getSystemTime_nanoseconds() { return duration_cast<nanoseconds>(system_clock::now().time_since_epoch()).count(); }

	inline double getSystemTime_microseconds() { return (double)getSystemTime_nanoseconds() / 1000.0; }

	inline double getSystemTime_milliseconds() { return (double)getSystemTime_nanoseconds() / 1000000.0; }

	inline double getSystemTime_seconds() { return (double)getSystemTime_nanoseconds() / 1000000000.0; }

};
