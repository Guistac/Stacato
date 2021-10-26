#include <pch.h>

#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

namespace EtherCatFieldbus {

	bool save(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		XMLElement* networkInterfaceCardXML = xml->InsertNewChildElement("NetworkInterfaceCard");
		XMLElement* primaryNicXML = networkInterfaceCardXML->InsertNewChildElement("Primary");
		primaryNicXML->SetAttribute("Description", networkInterfaceCard.description);
		primaryNicXML->SetAttribute("Name", networkInterfaceCard.name);
		if (EtherCatFieldbus::b_redundant) {
			XMLElement* redundantNicXML = networkInterfaceCardXML->InsertNewChildElement("Redundant");
			redundantNicXML->SetAttribute("Description", redundantNetworkInterfaceCard.description);
			redundantNicXML->SetAttribute("Name", redundantNetworkInterfaceCard.name);
		}
		XMLElement* timingXML = xml->InsertNewChildElement("Timing");
		timingXML->SetAttribute("ProcessIntervalMilliseconds", processInterval_milliseconds);
		timingXML->SetAttribute("ProcessDataTimeoutMilliseconds", processDataTimeout_milliseconds);
		timingXML->SetAttribute("ClockStableThresholdMilliseconds", clockStableThreshold_milliseconds);

		return true;
	}

	bool load(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		Logger::debug("Loading Fieldbus Parameters");

		XMLElement* networkInterfaceCardXML = xml->FirstChildElement("NetworkInterfaceCard");
		if (!networkInterfaceCardXML) return Logger::warn("Could not load Network Interface Card Attribute");

		XMLElement* primaryNicXML = networkInterfaceCardXML->FirstChildElement("Primary");
		if (!primaryNicXML) return Logger::warn("Could not load Primary Network Interface Card Attribute");
		const char* nicDescription;
		const char* nicName;
		if (primaryNicXML->QueryStringAttribute("Description", &nicDescription) != XML_SUCCESS) return Logger::warn("Could not read primary NIC description");
		if (primaryNicXML->QueryStringAttribute("Name", &nicName) != XML_SUCCESS) return Logger::warn("Could not read primary NIC name");
		strcpy(networkInterfaceCard.description, nicDescription);
		strcpy(networkInterfaceCard.name, nicName);

		const char* redundantNicDescription;
		const char* redundantNicName;
		XMLElement* redundantNicXML = networkInterfaceCardXML->FirstChildElement("Redundant");
		bool hasRedundantNic = redundantNicXML != nullptr;
		if (hasRedundantNic) {
			if (redundantNicXML->QueryStringAttribute("Description", &redundantNicDescription) != XML_SUCCESS) return Logger::warn("Could not read redundant NIC description");
			if (redundantNicXML->QueryStringAttribute("Name", &redundantNicName) != XML_SUCCESS) return Logger::warn("Could not read redundant NIC name");
			b_redundant = true;
			strcpy(redundantNetworkInterfaceCard.description, redundantNicDescription);
			strcpy(redundantNetworkInterfaceCard.name, redundantNicName);
		}

		XMLElement* timingXML = xml->FirstChildElement("Timing");
		if (timingXML->QueryDoubleAttribute("ProcessIntervalMilliseconds", &processInterval_milliseconds) != XML_SUCCESS) return Logger::warn("Could not load process interval value");
		if (timingXML->QueryDoubleAttribute("ProcessDataTimeoutMilliseconds", &processDataTimeout_milliseconds) != XML_SUCCESS) return Logger::warn("Could not load process data timeout value");
		if (timingXML->QueryDoubleAttribute("ClockStableThresholdMilliseconds", &clockStableThreshold_milliseconds) != XML_SUCCESS) return Logger::warn("Coult not load clock stable threshold value");
		
		Logger::info("Successfully Loaded Fieldbus Parameters");

		return true;
	}

}