#include <pch.h>

#include "Manoeuvre.h"
#include "ParameterTrack.h"

#include "Motion/Machine/Machine.h"
#include "Motion/AnimatableParameter.h"
#include "Environnement/Environnement.h"

#include <tinyxml2.h>


bool Manoeuvre::save(tinyxml2::XMLElement* manoeuvreXML) {
	using namespace tinyxml2;
	manoeuvreXML->SetAttribute("Name", name);
	manoeuvreXML->SetAttribute("Description", description);
	manoeuvreXML->SetAttribute("Type", Enumerator::getSaveString(type));
	for (auto& track : tracks) {
		//tracks which have a group parent are listed in the manoeuvres track vector
		//but they don't get saved in the main manoeuvre
		//instead they are saved by their parent parameter track
		if (track->hasParentParameterTrack()) continue;
		XMLElement* trackXML = manoeuvreXML->InsertNewChildElement("Track");
		track->save(trackXML);
	}
	return true;
}

bool Manoeuvre::load(tinyxml2::XMLElement* manoeuvreXML) {
	using namespace tinyxml2;
	const char* nameString;
	if (manoeuvreXML->QueryStringAttribute("Name", &nameString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Name Attribute");
	strcpy(name, nameString);
	const char* descriptionString;
	if (manoeuvreXML->QueryStringAttribute("Description", &descriptionString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre Description Attribute");
	strcpy(description, descriptionString);
	const char* manoeuvreTypeString;
	if (manoeuvreXML->QueryStringAttribute("Type", &manoeuvreTypeString) != XML_SUCCESS) return Logger::warn("Could not find Manoeuvre type");
	if (!Enumerator::isValidSaveName<Manoeuvre::Type>(manoeuvreTypeString)) return Logger::warn("Could not identify Manoeuvre type");
	type = Enumerator::getEnumeratorFromSaveString<Manoeuvre::Type>(manoeuvreTypeString);

	XMLElement* trackXML = manoeuvreXML->FirstChildElement("Track");
	while (trackXML != nullptr) {
		std::shared_ptr<ParameterTrack> track = std::make_shared<ParameterTrack>(shared_from_this());
		if (!track->load(trackXML)) return false;
		tracks.push_back(track);
		if (track->hasChildParameterTracks()) {
			for (auto& childParameterTrack : track->childParameterTracks) {
				childParameterTrack->parentParameterTrack = track;
				tracks.push_back(childParameterTrack);
			}
		}
		trackXML = trackXML->NextSiblingElement("Track");
	}

	return true;
}


bool ParameterTrack::save(tinyxml2::XMLElement* trackXML) {
	using namespace tinyxml2;
	
	trackXML->SetAttribute("Machine", parameter->machine->getName());
	trackXML->SetAttribute("MachineUniqueID", parameter->machine->getUniqueID());
	trackXML->SetAttribute("Parameter", parameter->name);

	trackXML->SetAttribute("HasChildParameterTracks", hasChildParameterTracks());

	if (hasChildParameterTracks()) {
		XMLElement* childParameterTracksXML = trackXML->InsertNewChildElement("ChildParameterTracks");
		for (auto& childParameterTrack : childParameterTracks) {
			XMLElement* childParameterTrackXML = childParameterTracksXML->InsertNewChildElement("ParameterTrack");
			if (!childParameterTrack->save(childParameterTrackXML)) return false;
		}
		return true;
	}

	trackXML->SetAttribute("SequenceType", getSequenceType(sequenceType)->saveName);
	trackXML->SetAttribute("Interpolation", Enumerator::getSaveString(interpolationType));

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
	case SequenceType::Type::CONSTANT:
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

	bool hasChildParameterTracks = false;
	if (trackXML->QueryBoolAttribute("HasChildParameterTracks", &hasChildParameterTracks) != XML_SUCCESS) return Logger::warn("Could not detect read Parameter Track Atribute");

	if (hasChildParameterTracks) {
		XMLElement* childParameterTracksXML = trackXML->FirstChildElement("ChildParameterTracks");
		if (childParameterTracksXML == nullptr) return Logger::warn("Could not find child parameter tracks attribute");
		XMLElement* childParameterTrackXML = childParameterTracksXML->FirstChildElement("ParameterTrack");
		while (childParameterTrackXML != nullptr) {
			std::shared_ptr<ParameterTrack> childParameterTrack = std::make_shared<ParameterTrack>(parentManoeuvre);
			if (!childParameterTrack->load(childParameterTrackXML)) return Logger::warn("Could not load child parameter Track");
			childParameterTracks.push_back(childParameterTrack);
			childParameterTrackXML = childParameterTrackXML->NextSiblingElement("ParameterTrack");
		}
		return true;
	}

	const char* interpolationTypeString;
	if (trackXML->QueryStringAttribute("Interpolation", &interpolationTypeString) != XML_SUCCESS) return Logger::warn("Could not find interpolation type attribute");
	if (!Enumerator::isValidSaveName<Motion::InterpolationType>(interpolationTypeString)) return Logger::warn("Could not read interpolation type");
	setInterpolationType(Enumerator::getEnumeratorFromSaveString<Motion::InterpolationType>(interpolationTypeString));

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
		if (!targetIsNextOrigin) {
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
	case SequenceType::Type::CONSTANT:
		if (targetIsNextOrigin) trackXML->SetAttribute("TargetIsNextOrigin", targetIsNextOrigin);
		if (!targetIsNextOrigin) {
			XMLElement* targetXML = trackXML->FirstChildElement("Target");
			if (targetXML == nullptr) return Logger::warn("Could not find target attribute");
			target.load(targetXML);
			origin = target;
		}
		break;
	}

	return true;
}
