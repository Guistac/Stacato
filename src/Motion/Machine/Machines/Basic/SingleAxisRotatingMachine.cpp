#include <pch.h>

#include "SingleAxisRotatingMachine.h"

void SingleAxisRotatingMachine::controlsGui() {}
void SingleAxisRotatingMachine::settingsGui() {}
void SingleAxisRotatingMachine::axisGui() {}
void SingleAxisRotatingMachine::deviceGui() {}
void SingleAxisRotatingMachine::metricsGui() {}
void SingleAxisRotatingMachine::miniatureGui() {}
bool SingleAxisRotatingMachine::isEnabled() { return false; }
bool SingleAxisRotatingMachine::isReady() { return false; }
void SingleAxisRotatingMachine::enable() {}
void SingleAxisRotatingMachine::disable() {}
bool SingleAxisRotatingMachine::isMoving() { return false; }

void SingleAxisRotatingMachine::assignIoData() {
	addIoData(axisLink);
}
void SingleAxisRotatingMachine::process() {}
