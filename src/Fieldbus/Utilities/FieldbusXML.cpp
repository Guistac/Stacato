#include <pch.h>

#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

namespace EtherCatFieldbus {

	bool save(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		XMLElement* networkInterfaceCardXML = xml->InsertNewChildElement("NetworkInterfaceCard");
		XMLElement* primaryNicXML = networkInterfaceCardXML->InsertNewChildElement("Primary");
		primaryNicXML->SetAttribute("Description", networkInterfaceCard.description);
		primaryNicXML->SetAttribute("Name", EtherCatFieldbus::networkInterfaceCard.name);
		if (EtherCatFieldbus::b_redundant) {
			XMLElement* redundantNicXML = networkInterfaceCardXML->InsertNewChildElement("Redundant");
			redundantNicXML->SetAttribute("Description", EtherCatFieldbus::redundantNetworkInterfaceCard.description);
			redundantNicXML->SetAttribute("Name", EtherCatFieldbus::redundantNetworkInterfaceCard.name);
		}
		XMLElement* timingXML = xml->InsertNewChildElement("Timing");
		timingXML->SetAttribute("ProcessIntervalMilliseconds", EtherCatFieldbus::processDataTimeout_milliseconds);
		timingXML->SetAttribute("ProcessDataTimeoutMilliseconds", EtherCatFieldbus::processDataTimeout_milliseconds);
		timingXML->SetAttribute("ClockStableThresholdMilliseconds", EtherCatFieldbus::clockStableThreshold_milliseconds);
		timingXML->SetAttribute("SlaveStateCheckCycleCount", EtherCatFieldbus::slaveStateCheckCycleCount);

		return true;
	}

	bool load(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		Logger::debug("Loading Fieldbus Parameters");

		XMLElement* networkInterfaceCardXML = xml->FirstChildElement("NetworkInterfaceCard");
		if (!networkInterfaceCardXML) return Logger::warn("Could not load Network Interface Card Attribute");
		XMLElement* primaryNicXML = networkInterfaceCardXML->FirstChildElement("Primary");
		if (!primaryNicXML) return Logger::warn("Could not load Primary Network Interface Card Attribute");
		const char* primaryNicDescription;
		const char* primaryNicName;
		if (primaryNicXML->QueryStringAttribute("Description", &primaryNicDescription) != XML_SUCCESS) return Logger::warn("Could not read primary NIC description");
		if (primaryNicXML->QueryStringAttribute("Name", &primaryNicName) != XML_SUCCESS) return Logger::warn("Could not read primary NIC name");
		XMLElement* redundantNicXML = networkInterfaceCardXML->FirstChildElement("Redundant");
		const char* redundantNicDescription;
		const char* redundantNicName;
		bool hasRedundantNic = redundantNicXML != nullptr;
		if (hasRedundantNic) {
			if (redundantNicXML->QueryStringAttribute("Description", &redundantNicDescription) != XML_SUCCESS) return Logger::warn("Could not read redundant NIC description");
			if (redundantNicXML->QueryStringAttribute("Name", &redundantNicName) != XML_SUCCESS) return Logger::warn("Could not read redundant NIC name");
		}

		XMLElement* timingXML = xml->FirstChildElement("Timing");
		double processIntervalMilliseconds;
		if (timingXML->QueryDoubleAttribute("ProcessIntervalMilliseconds", &processIntervalMilliseconds) != XML_SUCCESS) return Logger::warn("Could not load process interval value");
		double processDataTimeoutMilliseconds;
		if (timingXML->QueryDoubleAttribute("ProcessDataTimeoutMilliseconds", &processDataTimeoutMilliseconds) != XML_SUCCESS) return Logger::warn("Could not load process data timeout value");
		double clockStableThresholdMilliseconds;
		if (timingXML->QueryDoubleAttribute("ClockStableThresholdMilliseconds", &clockStableThresholdMilliseconds) != XML_SUCCESS) return Logger::warn("Coult not load clock stable threshold value");
		int slaveStateCheckCycleCount;
		if (timingXML->QueryIntAttribute("SlaveStateCheckCycleCount", &slaveStateCheckCycleCount) != XML_SUCCESS) return Logger::warn("Couldnot load slave state check cycle count value");
		
		Logger::info("Successfully Loaded Fieldbus Parameters");

		return true;
	}

}