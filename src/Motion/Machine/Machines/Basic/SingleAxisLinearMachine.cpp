#include <pch.h>

#include "SingleAxisLinearMachine.h"

void SingleAxisLinearMachine::controlsGui() {}
void SingleAxisLinearMachine::settingsGui() {}
void SingleAxisLinearMachine::axisGui() {}
void SingleAxisLinearMachine::deviceGui() {}
void SingleAxisLinearMachine::metricsGui(){}
void SingleAxisLinearMachine::miniatureGui() {}
bool SingleAxisLinearMachine::isEnabled() { return false; }
bool SingleAxisLinearMachine::isReady() { return false; }
void SingleAxisLinearMachine::enable() {}
void SingleAxisLinearMachine::disable() {}
bool SingleAxisLinearMachine::isMoving() { return false; }

void SingleAxisLinearMachine::assignIoData() {
	addIoData(axisLink);
}
void SingleAxisLinearMachine::process() {}