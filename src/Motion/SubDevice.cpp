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