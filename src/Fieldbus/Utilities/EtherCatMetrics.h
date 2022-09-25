#pragma once

#include "Fieldbus/EtherCatFieldbus.h"
#include "Utilities/ScrollingBuffer.h"

class EtherCatMetrics {
public:

	void init(double processInterval_milliseconds) {
		int scrollingBufferSize = scrollingBufferLength_seconds * 1000.0 / processInterval_milliseconds;
		dcTimeErrors.clear();
		averageDcTimeErrors.clear();
		workingCounters.clear();
		sendDelays.clear();
		receiveDelays.clear();
		timeoutDelays.clear();
		timeouts.clear();
		processDelays.clear();
		cycleLengths.clear();
		dcTimeErrors.setMaxSize(scrollingBufferSize);
		averageDcTimeErrors.setMaxSize(scrollingBufferSize);
		workingCounters.setMaxSize(scrollingBufferSize);
		sendDelays.setMaxSize(scrollingBufferSize);
		receiveDelays.setMaxSize(scrollingBufferSize);
		timeoutDelays.setMaxSize(scrollingBufferSize);
		timeouts.setMaxSize(scrollingBufferSize);
		processDelays.setMaxSize(scrollingBufferSize);
		cycleLengths.setMaxSize(scrollingBufferSize);
		cycleCounter = 0;
		frameReturnTypeCounters[0] = 0;
		frameReturnTypeCounters[1] = 0;
        frameCount = 0;
        droppedFrameCount = 0;
	}

	uint64_t cycleCounter;
	double fieldbusTime_seconds;
	float averageDcTimeError_milliseconds; //latest reference clock error, used to track clock sync progress when starting the fieldbus

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

    unsigned long long frameCount = 0;
    unsigned long long droppedFrameCount = 0;
    
	//working counter results
	int frameReturnTypeCounters[2] = { 0, 0 };
	const char* frameReturnTypeChars[2] = {
		"Healthy",
		"Timeout"
	};

	void addWorkingCounter(int workingCounter, double time) {
		if (workingCounters.size() == workingCounters.maxSize()) {
			int oldestWorkingCounter = workingCounters.oldest().y;
			if (oldestWorkingCounter > 0) frameReturnTypeCounters[0]--;
			else frameReturnTypeCounters[1]--;
		}
		workingCounters.addPoint(glm::vec2(time, workingCounter));
		if (workingCounter > 0) frameReturnTypeCounters[0]++;
		else frameReturnTypeCounters[1]++;
	}
};
