#pragma once

#include <chrono>

class Timing {
public:

	static std::chrono::system_clock::time_point programStart;

	static void start() {
		programStart = std::chrono::system_clock::now();
	}

	static double getTime_seconds() {
		std::chrono::duration delay = std::chrono::system_clock::now() - programStart;
		std::chrono::nanoseconds delayNanoseconds = std::chrono::duration_cast<std::chrono::nanoseconds>(delay);
		return (double)delayNanoseconds.count() / 1000000000.0L;
	}

};