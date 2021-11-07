#include <pch.h>

#include "ParameterTrack.h"

#include "Motion/Machine/Machine.h"
#include "Motion/AnimatableParameter.h"
#include "Project/Environnement.h"

#include <tinyxml2.h>

bool ParameterTrack::save(tinyxml2::XMLElement* trackXML) {
	using namespace tinyxml2;
	
	trackXML->SetAttribute("Machine", parameter->machine->getName());
	trackXML->SetAttribute("MachineUniqueID", parameter->machine->getUniqueID());
	trackXML->SetAttribute("Parameter", parameter->name);
	trackXML->SetAttribute("SequenceType", getSequenceType(sequenceType)->saveName);
	trackXML->SetAttribute("Interpolation", getInterpolationType(interpolationType)->saveName);

	switch (sequenceType) {
	case SequenceType::Type::ANIMATED_MOVE:
		trackXML->SetAttribute("OriginIsPreviousTarget", originIsPreviousTarget);
		trackXML->SetAttribute("TargetIsNextOrigin", targetIsNextOrigin);
		
		//for (auto& curve : curves) {
		//	XMLElement* curveXML = trackXML->InsertNewChildElement("Curve");
		//	curve->save(curveXML);
		//}
		
		break;
	case SequenceType::Type::TIMED_MOVE:
		trackXML->SetAttribute("OriginIsPreviousTarget", originIsPreviousTarget);
		if(!originIsPreviousTarget) {
			XMLElement* originXML = trackXML->InsertNewChildElement("Origin");
			origin.save(originXML);
		}
		trackXML->SetAttribute("TargetIsNextOrigin", targetIsNextOrigin);
		if(!targetIsNextOrigin) {
			XMLElement* targetXML = trackXML->InsertNewChildElement("Target");
			target.save(targetXML);
		}
		trackXML->SetAttribute("MovementTime", movementTime);
		trackXML->SetAttribute("TimeOffset", timeOffset);
		trackXML->SetAttribute("RampIn", rampIn);
		trackXML->SetAttribute("RampsAreEqual", rampsAreEqual);
		if (!rampsAreEqual) trackXML->SetAttribute("RampOut", rampOut);
		break;
	case SequenceType::Type::NO_MOVE:
		XMLElement* targetXML = trackXML->InsertNewChildElement("Target");
		targetXML->SetAttribute("IsEqualToNextOrigin", targetIsNextOrigin);
		if(!targetIsNextOrigin) target.save(targetXML);
		break;
	}
	return true;
}

bool ParameterTrack::load(tinyxml2::XMLElement* trackXML) {
	using namespace tinyxml2;

	const char* machineNameString;
	if (trackXML->QueryStringAttribute("Machine", &machineNameString) != XML_SUCCESS) return Logger::warn("Could not find machine attribute");
	int machineID;
	if (trackXML->QueryIntAttribute("MachineUniqueID", &machineID) != XML_SUCCESS) return Logger::warn("Could not find machine unique ID");

	std::shared_ptr<Machine> machine = nullptr;
	for (auto& m : Environnement::getMachines()) {
		if (strcmp(machineNameString, m->getName()) == 0 && machineID == m->getUniqueID()) {
			machine = m;
			break;
		}
	}
	if (machine == nullptr) return Logger::warn("Could not identify machine {} with unique ID {}", machineNameString, machineID);

	const char* parameterNameString;
	if (trackXML->QueryStringAttribute("Parameter", &parameterNameString) != XML_SUCCESS) return Logger::warn("Could not find parameter name attribute");
	parameter = nullptr;
	for (auto& p : machine->animatableParameters) {
		if (strcmp(parameterNameString, p->name) == 0) {
			parameter = p;
			break;
		}
	}
	if (parameter == nullptr) return Logger::warn("Could not find parameter named {}", parameterNameString);

	//after we got the parameter object from the right machine, we can initialize the track
	initialize();

	const char* interpolationTypeString;
	if (trackXML->QueryStringAttribute("Interpolation", &interpolationTypeString) != XML_SUCCESS) return Logger::warn("Could not find interpolation type attribute");
	if (getInterpolationType(interpolationTypeString) == nullptr) return Logger::warn("Could not read interpolation type");
	setInterpolationType(getInterpolationType(interpolationTypeString)->type);

	const char* sequenceTypeString;
	trackXML->QueryStringAttribute("SequenceType", &sequenceTypeString);
	if (getSequenceType(sequenceTypeString) == nullptr) return Logger::warn("Could not identify paramete track sequence type");
	setSequenceType(getSequenceType(sequenceTypeString)->type);

	switch (sequenceType) {
	case SequenceType::Type::ANIMATED_MOVE:

		if (trackXML->QueryBoolAttribute("OriginIsPreviousTarget", &originIsPreviousTarget) != XML_SUCCESS) return Logger::warn("Could not read target equal");
		if (trackXML->QueryBoolAttribute("TargetIsNextOrigin", &targetIsNextOrigin) != XML_SUCCESS) return Logger::warn("Could not read origin equal");
		//load curves
		//for (auto& curve : curves) {
			//XMLElement* curveXML = trackXML->FirstChildElement("Curve");
			//curve->save(curveXML);
		//}
		break;
	case SequenceType::Type::TIMED_MOVE:
		if (trackXML->QueryBoolAttribute("OriginIsPreviousTarget", &originIsPreviousTarget) != XML_SUCCESS) return Logger::warn("Could not read target equal");
		if (!originIsPreviousTarget) {
			XMLElement* originXML = trackXML->FirstChildElement("Origin");
			if (originXML == nullptr) return Logger::warn("Could not find origin attribute");
			origin.load(originXML);
		}
		if (trackXML->QueryBoolAttribute("TargetIsNextOrigin", &targetIsNextOrigin) != XML_SUCCESS) return Logger::warn("Could not read origin equal");
		if (targetIsNextOrigin) {
			XMLElement* targetXML = trackXML->FirstChildElement("Target");
			if (targetXML == nullptr) return Logger::warn("Could not find target attribute");
			target.load(targetXML);
		}
		if (trackXML->QueryDoubleAttribute("MovementTime", &movementTime) != XML_SUCCESS) return Logger::warn("Could not read Movement Time Attribute");
		if (trackXML->QueryDoubleAttribute("TimeOffset", &timeOffset) != XML_SUCCESS) return Logger::warn("Could not read Time Offset Attribute");
		if (trackXML->QueryDoubleAttribute("RampIn", &rampIn) != XML_SUCCESS) return Logger::warn("Could not read Ramp In Attribute");
		if (trackXML->QueryBoolAttribute("RampsAreEqual", &rampsAreEqual) != XML_SUCCESS) return Logger::warn("Could not read ramp Equal attribute");
		if (!rampsAreEqual) {
			if (trackXML->QueryDoubleAttribute("RampOut", &rampOut) != XML_SUCCESS) return Logger::warn("Could not read ramp out attribute");
		}
		break;
	case SequenceType::Type::NO_MOVE:
		if (targetIsNextOrigin) trackXML->SetAttribute("TargetIsNextOrigin", targetIsNextOrigin);
		if (!targetIsNextOrigin) {
			XMLElement* targetXML = trackXML->FirstChildElement("Target");
			if (targetXML == nullptr) return Logger::warn("Could not find target attribute");
			target.load(targetXML);
		}
		break;
	}
	
	return true;
}