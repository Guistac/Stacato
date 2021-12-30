#include <pch.h>

#include "OscDevice.h"

#include "Networking/Network.h"

#include <imgui.h>
#include <tinyxml2.h>
#include "OscSocket.h"

#include "Gui/Assets/Fonts.h"

void OscDevice::assignIoData(){}

void OscDevice::connect(){
	oscSocket = std::make_shared<OscSocket>(4096);
	oscSocket->open(listeningPort, std::vector<int>({remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3]}), remotePort);
	
	if(!oscSocket->isOpen()) return Logger::error("{} : Could not start Osc Socket", getName());
	Logger::info("{} : Started Osc Socket, Remote Address: {}.{}.{}.{}:{}  Listening Port: {}", getName(), remoteIP[0], remoteIP[1], remoteIP[2], remoteIP[3], remotePort, listeningPort);
	
	b_enabled = true;
	runtime = std::thread([this](){
		Logger::critical("START OSC THREAD");
		while(b_enabled){
			
			std::shared_ptr<OscMessage> message = std::make_shared<OscMessage>("/Stacato/TestMessage");
			message->addFloat(Timing::getProgramTime_seconds());
			oscSocket->send(message);
			
			Logger::warn("{}", Timing::getProgramTime_seconds());
			std::this_thread::sleep_for(std::chrono::milliseconds(50));
		}
		Logger::critical("EXIT OSC THREAD");
	});
	
}

void OscDevice::disconnect(){
	b_enabled = false;
	runtime.join();
	oscSocket = nullptr;
	Logger::info("{} : Closed Osc Socket", getName());
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
