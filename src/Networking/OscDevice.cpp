#include <pch.h>

#include "OscDevice.h"

#include "Network.h"

#include <imgui.h>
#include <tinyxml2.h>
#include "OscSocket.h"

#include "Gui/Assets/Fonts.h"

void OscDevice::assignIoData(){}

void OscDevice::connect(){
	oscSocket = std::make_shared<OscSocket>(4096);
	oscSocket->open(listeningPort, std::vector<int>({remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]}), remotePort);
}

void OscDevice::disconnect(){
	oscSocket = nullptr;
}

bool OscDevice::isConnected(){}

bool OscDevice::isDetected(){}

void OscDevice::onConnection(){}

void OscDevice::onDisconnection(){}

void OscDevice::readInputs(){}

void OscDevice::prepareOutputs(){}

void OscDevice::nodeSpecificGui(){
	if(ImGui::BeginTabItem("Network")){
		networkGui();
		ImGui::EndTabItem();
	}
	if(ImGui::BeginTabItem("Data")){
		dataGui();
		ImGui::EndTabItem();
	}
}

void OscDevice::networkGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("UDP Socket");
	ImGui::PopFont();
	ImGui::Text("Remote IP");
	ImGui::InputInt4("##RemoteIP", remoteIP);
	ImGui::Text("Remote Port");
	ImGui::InputInt("##RemotePort", &remotePort);
	ImGui::Text("Listening Port");
	ImGui::InputInt("##ListeningPort", &listeningPort);
}

void OscDevice::dataGui(){
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Outgoing Messages");
	ImGui::PopFont();
	
	ImGui::Separator();
	
	ImGui::PushFont(Fonts::robotoBold20);
	ImGui::Text("Incoming Messages");
	ImGui::PopFont();
}


bool OscDevice::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* remoteIPXML = xml->InsertNewChildElement("RemoteIP");
	remoteIPXML->SetAttribute("ipv4-0", remoteIP[0]);
	remoteIPXML->SetAttribute("ipv4-1", remoteIP[1]);
	remoteIPXML->SetAttribute("ipv4-2", remoteIP[2]);
	remoteIPXML->SetAttribute("ipv4-3", remoteIP[3]);
	
	XMLElement* portsXML = xml->InsertNewChildElement("Ports");
	portsXML->SetAttribute("send", remotePort);
	portsXML->SetAttribute("receive", listeningPort);
	
	return true;
}


bool OscDevice::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;

	XMLElement* remoteIPXML = xml->FirstChildElement("RemoteIP");
	if(remoteIPXML == nullptr) return Logger::warn("Could not find remote ip attribute");
	if(remoteIPXML->QueryAttribute("ipv4-0", &remoteIP[0]) != XML_SUCCESS) return Logger::warn("could not find ipv4#0 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-1", &remoteIP[1]) != XML_SUCCESS) return Logger::warn("could not find ipv4#1 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-2", &remoteIP[2]) != XML_SUCCESS) return Logger::warn("could not find ipv4#2 attribute");
	if(remoteIPXML->QueryAttribute("ipv4-3", &remoteIP[3]) != XML_SUCCESS) return Logger::warn("could not find ipv4#3 attribute");
	
	XMLElement* portsXML = xml->FirstChildElement("Ports");
	if(portsXML == nullptr) return Logger::warn("Could not find port attribute");
	if(portsXML->QueryAttribute("send", &remotePort) != XML_SUCCESS) return Logger::warn("Could not find send port attribute");
	if(portsXML->QueryAttribute("receive", &listeningPort) != XML_SUCCESS) return Logger::warn("Could not find receive port attribute");
	
	return true;
}
