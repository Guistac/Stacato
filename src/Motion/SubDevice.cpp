#include <pch.h>
#include "SubDevice.h"

#include "NodeGraph/Device.h"

bool Subdevice::isDetected() {
	//if (parentDevice == nullptr) return false;
	//return (parentDevice->isDetected() && b_detected);
	return b_detected;
}

bool Subdevice::isOnline() {
	/*
	if (parentDevice == nullptr) return false;
	return (parentDevice->isOnline() && b_online);
	*/
	return b_online;
}

bool Subdevice::isReady() {
	/*
	if (parentDevice == nullptr) return false;
	return (parentDevice->isReady() && b_ready);
	*/
	return b_ready;
}