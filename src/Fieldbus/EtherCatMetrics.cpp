#include "EtherCatMetrics.h"

#include "EtherCatFieldbus.h"

#include <iostream>

void EtherCatMetrics::init() {
	int scrollingBufferSize = scrollingBufferLength_seconds * 1000.0 / EtherCatFieldbus::processInterval_milliseconds;
	dcTimeErrors.setMaxSize(scrollingBufferSize);
	averageDcTimeErrors.setMaxSize(scrollingBufferSize);
	workingCounters.setMaxSize(scrollingBufferSize);
	sendDelays.setMaxSize(scrollingBufferSize);
	receiveDelays.setMaxSize(scrollingBufferSize);
	timeoutDelays.setMaxSize(scrollingBufferSize);
	timeouts.setMaxSize(scrollingBufferSize);
	processDelays.setMaxSize(scrollingBufferSize);
	cycleLengths.setMaxSize(scrollingBufferSize);
}

void EtherCatMetrics::reset() {
	dcTimeErrors.clear();
	averageDcTimeErrors.clear();
	workingCounters.clear();
	sendDelays.clear();
	receiveDelays.clear();
	timeoutDelays.clear();
	timeouts.clear();
	processDelays.clear();
	cycleLengths.clear();
	cycleCounter = 0;
	frameReturnTypeCounters[0] = 0;
	frameReturnTypeCounters[1] = 0;
}

void EtherCatMetrics::addWorkingCounter(int workingCounter, double time) {
	if (workingCounters.size() == workingCounters.maxSize()) {
		int oldestWorkingCounter = workingCounters.oldest().y;
		if (oldestWorkingCounter > 0) frameReturnTypeCounters[0]--;
		else frameReturnTypeCounters[1]--;
	}
	workingCounters.addPoint(glm::vec2(time, workingCounter));
	if (workingCounter > 0) frameReturnTypeCounters[0]++;
	else frameReturnTypeCounters[1]++;
}