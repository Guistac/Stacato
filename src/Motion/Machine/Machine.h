#pragma once

#include "NodeGraph/Node.h"
#include "Motion/Subdevice.h"
#include "Motion/MotionTypes.h"
#include "Motion/MotionCurve.h"
#include "Utilities/CircularBuffer.h"

class Machine : public Node {
public:

	virtual void controlsGui() = 0;
	virtual void settingsGui() = 0;
	virtual void devicesGui() = 0;
	virtual void metricsGui() = 0;
	virtual void miniatureGui() = 0;

	virtual void nodeSpecificGui();

	//reference to stage geometry
	//reference to a parent axis

	bool isEnabled() { return b_isEnabled; }
	bool isReady() { return b_isReady; }
	void enable() { b_setEnable = true; }

private:

	bool b_setEnable = false;
	bool b_isReady = false;
	bool b_isEnabled = false;
};