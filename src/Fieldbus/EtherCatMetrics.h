#pragma once

#include "Utilities/ScrollingBuffer.h"

class EtherCatMetrics {
public:

	void init();
	void reset();

	uint64_t cycleCounter;
	uint64_t startTime_nanoseconds;

	uint64_t processTime_nanoseconds;
	double processTime_seconds;
	
	float averageDcTimeError_milliseconds;

	ScrollingBuffer dcTimeErrors;
	ScrollingBuffer averageDcTimeErrors;
	ScrollingBuffer workingCounters;
	ScrollingBuffer sendDelays;
	ScrollingBuffer receiveDelays;
	ScrollingBuffer timeoutDelays;
	ScrollingBuffer timeouts;
	ScrollingBuffer processDelays;
	ScrollingBuffer cycleLengths;
	double scrollingBufferLength_seconds = 30.0;


	//working counter results
	int frameReturnTypeCounters[2] = { 0, 0 };
	const char* frameReturnTypeChars[2] = {
		"Healthy",
		"Timeout"
	};

	void addWorkingCounter(int, double);
};