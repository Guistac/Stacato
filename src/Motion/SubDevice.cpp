#include <pch.h>
#include "SubDevice.h"

#include "NodeGraph/DeviceNode.h"

bool Subdevice::isDetected() {
	return (parentDevice->isDetected() && b_detected);
}

bool Subdevice::isOnline() {
	return (parentDevice->isOnline() && b_online);
}

bool Subdevice::isReady() {
	return (parentDevice->isReady() && b_ready);
}

bool Subdevice::hasError() {
	return (parentDevice->hasError() || b_error);
}

void Subdevice::clearError() {
	parentDevice->clearError();
	b_clearError = true;
}

const char* Subdevice::getErrorString() {
	if (b_error) return errorString;
	else if (parentDevice->hasError()) return parentDevice->getErrorString();
}