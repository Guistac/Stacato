#include <pch.h>

#include "Fieldbus/EtherCatFieldbus.h"

#include <tinyxml2.h>

namespace EtherCatFieldbus {

	bool save(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		/*
		XMLElement* networkInterfaceCardXML = xml->InsertNewChildElement("NetworkInterfaceCard");
		if(EtherCatFieldbus::hasNetworkInterface()){
			XMLElement* primaryNicXML = networkInterfaceCardXML->InsertNewChildElement("Primary");
			auto mainNic = getActiveNetworkInterfaceCard();
			primaryNicXML->SetAttribute("Description", mainNic->description);
			primaryNicXML->SetAttribute("Name", mainNic->name);
			if (EtherCatFieldbus::hasRedundantInterface()) {
				XMLElement* redundantNicXML = networkInterfaceCardXML->InsertNewChildElement("Redundant");
				auto redundantNic = getActiveRedundantNetworkInterfaceCard();
				redundantNicXML->SetAttribute("Description", redundantNic->description);
				redundantNicXML->SetAttribute("Name", redundantNic->name);
			}
		}
		 */
		
		XMLElement* timingXML = xml->InsertNewChildElement("Timing");
		timingXML->SetAttribute("ProcessIntervalMilliseconds", processInterval_milliseconds);
		timingXML->SetAttribute("ProcessDataTimeoutMilliseconds", processDataTimeout_milliseconds);
		timingXML->SetAttribute("ClockStableThresholdMilliseconds", clockStableThreshold_milliseconds);
		timingXML->SetAttribute("FieldbusTimeoutDelayMilliseconds", fieldbusTimeout_milliseconds);

		return true;
	}

	bool load(tinyxml2::XMLElement* xml) {
		using namespace tinyxml2;

		Logger::debug("Loading Fieldbus Parameters");
		
		/*
		EtherCatFieldbus::terminate();

		XMLElement* networkInterfaceCardXML = xml->FirstChildElement("NetworkInterfaceCard");
		if (!networkInterfaceCardXML) return Logger::warn("Could not load Network Interface Card Attribute");
		XMLElement* primaryNicXML = networkInterfaceCardXML->FirstChildElement("Primary");
		if (primaryNicXML){
			const char* primaryNicDescription = "";
			const char* redundantNicDescription = "";
			if (primaryNicXML->QueryStringAttribute("Description", &primaryNicDescription) != XML_SUCCESS) return Logger::warn("Could not read primary NIC description");
			XMLElement* redundantNicXML = networkInterfaceCardXML->FirstChildElement("Redundant");
			bool hasRedundantNic = redundantNicXML != nullptr;
			if (hasRedundantNic) {
				if (redundantNicXML->QueryStringAttribute("Description", &redundantNicDescription) != XML_SUCCESS) return Logger::warn("Could not read redundant NIC description");
			}
			setDefaultNetworkInterfaces(primaryNicDescription, redundantNicDescription);
		}
		 */

		XMLElement* timingXML = xml->FirstChildElement("Timing");
		if (timingXML->QueryIntAttribute("ProcessIntervalMilliseconds", &processInterval_milliseconds) != XML_SUCCESS) return Logger::warn("Could not load process interval value");
		if (timingXML->QueryDoubleAttribute("ProcessDataTimeoutMilliseconds", &processDataTimeout_milliseconds) != XML_SUCCESS) return Logger::warn("Could not load process data timeout value");
		if (timingXML->QueryDoubleAttribute("ClockStableThresholdMilliseconds", &clockStableThreshold_milliseconds) != XML_SUCCESS) return Logger::warn("Coult not load clock stable threshold value");
		if(timingXML->QueryDoubleAttribute("FieldbusTimeoutDelayMilliseconds", &fieldbusTimeout_milliseconds) != XML_SUCCESS) return Logger::warn("Coult not load fieldbus timeout delay value");
		
		Logger::info("Successfully Loaded Fieldbus Parameters");

		return true;
	}

}
