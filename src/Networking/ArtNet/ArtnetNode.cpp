#include <pch.h>

#include "ArtnetNode.h"

#include "Networking/Network.h"

#include "Scripting/Script.h"
#include "Scripting/EnvironnementLibrary.h"

#include <tinyxml2.h>

#include <ArtNode.h>

//Operating System Abstraction Layer from libSOEM
//Uselful for precise cross-platform thread sleep
#include <osal.h>

static int bufferSize = 1024;

void ArtNetNode::initialize(){
	
	script = std::make_shared<LuaScript>("PSN Server Script");
	script->setLoadLibrairiesCallback([](lua_State* L){
		Scripting::EnvironnementLibrary::openlib(L);
		//Scripting::ArtNetLibrary::openlib(L);
	});
	ArtConfig config;
}

void ArtNetNode::connect(){
		
	//configuration object is useless for the simple dmx packets we want to send
	ArtConfig configuration;
	buffer.resize(bufferSize, 0);
	node = std::make_shared<ArtNode>(configuration, bufferSize, buffer.data());
	
	//we are sending artnet packets over udp
	udpSocket = Network::getUdpSocket(123, {123,234,134,125}, 12348);
	if(udpSocket == nullptr){
		b_running = false;
		return;
	}
	
	start();
}


void ArtNetNode::start(){
	b_running = true;
	std::thread dmxThread([this](){
		
		long long waitTimeMicros = 1000000.0 / sendingFrequency->value;
		uint8_t net = (universe->value >> 8) & 0xFF;
		uint8_t subUni = universe->value & 0xFF;
		
		int counter = 0;
		
		while(b_running){
			
			ArtDmx* dmx = node->createDmx();
			dmx->Net = net;
			dmx->SubUni = subUni;
			for(int i = 0; i < 512; i++) dmx->Data[i] = counter + i;
			counter++;
			
			unsigned char* bufferData = node->getBufferData();
			size_t bufferDataSize = sizeof(ArtDmx);
			
			try {
				udpSocket->async_send(asio::buffer(bufferData, bufferDataSize), [](asio::error_code error, size_t byteCount) {
					if (error) Logger::debug("Failed to send ArtNet DMX Message: {}", error.message());
				});
			}
			catch (std::exception e) {
				Logger::error("Failed to start async_send: {}", e.what());
			}
			osal_usleep(waitTimeMicros);
		}
	});
	dmxThread.detach();
}

void ArtNetNode::stop(){
	b_running = false;
	udpSocket = nullptr;
}




void ArtNetNode::disconnect(){
	stop();
}

bool ArtNetNode::isConnected(){
	return b_running;
}

bool ArtNetNode::isDetected(){
	return Network::isInitialized();
}

void ArtNetNode::onConnection(){
}

void ArtNetNode::onDisconnection(){
}

void ArtNetNode::readInputs(){
	//incoming messages should be read by and asynchronous asio routine
}

void ArtNetNode::writeOutputs(){
	//outgoing messages should be sent by their respective timing thread
}





bool ArtNetNode::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* settingsXML = xml->InsertNewChildElement("Settings");
	ipAddress0->save(settingsXML);
	ipAddress1->save(settingsXML);
	ipAddress2->save(settingsXML);
	ipAddress3->save(settingsXML);
	portNumber->save(settingsXML);
	sendingFrequency->save(settingsXML);
	universe->save(settingsXML);
	
	XMLElement* scriptXML = xml->InsertNewChildElement("Script");
	scriptXML->SetText(script->getScriptText().c_str());
	
	return true;
}


bool ArtNetNode::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* settingsXML;
	if(!loadXMLElement("Settings", xml, settingsXML)) return false;
	
	if(!ipAddress0->load(settingsXML)) return false;
	if(!ipAddress1->load(settingsXML)) return false;
	if(!ipAddress2->load(settingsXML)) return false;
	if(!ipAddress3->load(settingsXML)) return false;
	if(!portNumber->load(settingsXML)) return false;
	if(!sendingFrequency->load(settingsXML)) return false;
	if(!universe->load(settingsXML)) return false;
	
	XMLElement* scriptXML;
	if(!loadXMLElement("Script", xml, scriptXML)) return false;
	std::string scriptString = std::string(scriptXML->GetText());
	script->load(scriptString);
	
	return true;
}
