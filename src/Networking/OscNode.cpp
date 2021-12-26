#include <pch.h>

#include "OscNode.h"

#include <imgui.h>


bool OscNode::isOnline(){}
bool OscNode::isReady(){}
bool OscNode::isEnabled(){}
bool OscNode::hasError(){}
const char* OscNode::getErrorString(){}
void OscNode::enable(){}
void OscNode::disable(){}
void OscNode::clearError(){}

bool OscNode::isDetected(){}

void OscNode::readInputs(){}

void OscNode::prepareOutputs(){}

void OscNode::onConnection(){}

void OscNode::onDisconnection(){}

void OscNode::assignIoData(){}

void OscNode::nodeSpecificGui(){}

//TODO: remove: isReady isEnabled hasError getErrorString enable disable clearError
