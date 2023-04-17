#include <pch.h>

#include "ArtnetNode.h"

#include "Networking/Network.h"

#include "Scripting/Script.h"
#include "Scripting/ArtNetLibrary.h"
#include "Scripting/EnvironnementLibrary.h"

#include <tinyxml2.h>

#include <ArtNode.h>

//Operating System Abstraction Layer from libSOEM
//Uselful for precise cross-platform thread sleep
#include <osal.h>


DmxUniverse::DmxUniverse(uint16_t universeNumber){
	if(universeNumber > 32767) universeNumber = 32767;
	ArtConfig configuration;
	buffer.resize(bufferSize, 0);
	artnode = std::make_shared<ArtNode>(configuration, bufferSize, buffer.data());
	artdmx = artnode->createDmx();
	artdmx->Net = (universeNumber >> 8) & 0xFF;
	artdmx->SubUni = universeNumber & 0xFF;
}

void DmxUniverse::setChannel(int channel, uint8_t value){
	if(channel > 511) return;
	artdmx->Data[channel] = value;
}

uint8_t* DmxUniverse::getBuffer(){
	return buffer.data();
}

size_t DmxUniverse::getBufferSize(){
	return sizeof(ArtDmx);
}

std::shared_ptr<DmxUniverse> ArtNetNode::createNewUniverse(uint16_t universeNumber){
	auto newUniverse = std::make_shared<DmxUniverse>(universeNumber);
	universes.push_back(newUniverse);
	return newUniverse;
}

void ArtNetNode::removeAllUniverses(){
	universes.clear();
}



void ArtNetNode::onConstruction(){
	
	NetworkDevice::onConstruction();
	
	setName("ArtNet Node");
	
	script = std::make_shared<LuaScript>("PSN Server Script");
	script->setLoadLibrairiesCallback([this](lua_State* L){
		Scripting::EnvironnementLibrary::openlib(L);
		Scripting::ArtNetLibrary::openlib(L, std::dynamic_pointer_cast<ArtNetNode>(shared_from_this()));
	});
	
	std::string defaultScript =
		"\n--Default ArtNet DMX Node Script"
		"\n"
		"\nlocal universe = ArtNetNode:createNewUniverse(1)"
		"\n"
		"\nfunction setup()"
		"\n"
		"\n	universe:setChannel(1, 127)"
		"\n"
		"\nend"
		"\n"
		"\n"
		"\nfunction update()"
		"\n"
		"\nend"
		"\n";
	script->load(defaultScript);
	
	
	
	ipAddress0 = Legato::NumberParameter<uint8_t>::createInstance(192, "ArtNet IP Octet 0", "IpOctet0");
	ipAddress1 = Legato::NumberParameter<uint8_t>::createInstance(168, "ArtNet IP Octet 1", "IpOctet1");
	ipAddress2 = Legato::NumberParameter<uint8_t>::createInstance(1, "ArtNet IP Octet 2", "IpOctet2");
	ipAddress3 = Legato::NumberParameter<uint8_t>::createInstance(33, "ArtNet IP Octet 3", "IpOctet3");
	portNumber = Legato::NumberParameter<uint16_t>::createInstance(6454, "ArtNet Port Number", "Port");
	broadcast = Legato::BooleanParameter::createInstance(false, "ArtNet Broadcast", "Broadcast");
	
	networkIpAddress0 = Legato::NumberParameter<uint8_t>::createInstance(192, "ArtNet Network IP Octet 0", "NetworkIpOctet0");
	networkIpAddress1 = Legato::NumberParameter<uint8_t>::createInstance(168, "ArtNet Network IP Octet 1", "NetworkIpOctet1");
	networkIpAddress2 = Legato::NumberParameter<uint8_t>::createInstance(0, "ArtNet Network IP Octet 2", "NetworkIpOctet2");
	networkIpAddress3 = Legato::NumberParameter<uint8_t>::createInstance(0, "ArtNet Network IP Octet 3", "NetworkIpOctet3");
	networkMask0 = Legato::NumberParameter<uint8_t>::createInstance(255, "ArtNet Network Mask Octet 0", "NetworkMaskOctet0");
	networkMask1 = Legato::NumberParameter<uint8_t>::createInstance(255, "ArtNet Network Mask Octet 1", "NetworkMaskOctet1");
	networkMask2 = Legato::NumberParameter<uint8_t>::createInstance(255, "ArtNet Network Mask Octet 2", "NetworkMaskOctet2");
	networkMask3 = Legato::NumberParameter<uint8_t>::createInstance(0, "ArtNet Network Mask Octet 3", "NetworkMaskOctet3");
	
	sendingFrequency = Legato::NumberParameter<double>::createInstance(1.0, "Sending frequency", "SendingFrequency");
	sendingFrequency->setUnit(Units::Frequency::Hertz);
	sendingFrequency->allowNegatives(false);
	
	ArtConfig config;
}

void ArtNetNode::connect(){
	//we are sending artnet packets over udp (broadcast or not)
	if(broadcast->getValue()) udpSocket = Network::getUdpBroadcastSocket();
	else udpSocket = Network::getUdpSocket(0, {ipAddress0->getValue(), ipAddress1->getValue(), ipAddress2->getValue(), ipAddress3->getValue()}, portNumber->getValue());
	if(udpSocket){
		b_running = true;
        start();
    }else b_running = false;
}


void ArtNetNode::start(){
	b_running = true;
	std::thread dmxThread([this](){
		
		long long waitTimeMicros = 1000000.0 / sendingFrequency->getValue();
		removeAllUniverses();
		
		script->compileAndRun();
		script->callFunction("setup");
		
        uint32_t address_u32 = networkIpAddress0->getValue() << 24 | networkIpAddress1->getValue() << 16 | networkIpAddress2->getValue() << 8 | networkIpAddress3->getValue();
        uint32_t mask_u32 = networkMask0->getValue() << 24 | networkMask1->getValue() << 16 | networkMask2->getValue() << 8 | networkMask3->getValue();
        asio::ip::address_v4 address = asio::ip::make_address_v4(address_u32);
        asio::ip::address_v4 mask = asio::ip::make_address_v4(mask_u32);
        asio::ip::network_v4 network(address, mask);
        asio::ip::udp::endpoint broadcastEndpoint(network.broadcast(), portNumber->getValue());

		bool b_broadcast = broadcast->getValue();
		
		while(b_running){
			
			script->callFunction("update");
			
			for(auto universe : universes){
				uint8_t* buffer = universe->getBuffer();
				size_t bufferSize = universe->getBufferSize();
				try {
					
					if(b_broadcast){
						udpSocket->async_send_to(asio::buffer(buffer, bufferSize), broadcastEndpoint, [this](asio::error_code error, size_t byteCount){
							if(error) b_isSending = false;
							else b_isSending = true;
						});
					}else{
						udpSocket->async_send(asio::buffer(buffer, bufferSize), [this](asio::error_code error, size_t byteCount) {
							if(error) b_isSending = false;
							else b_isSending = true;
						});
					}
					
				}
				catch (std::exception e) {
					Logger::error("Failed to start async_send: {}", e.what());
				}
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
	return b_running && b_isSending;
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





bool ArtNetNode::onSerialization(){
	bool success = true;
	success &= NetworkDevice::onSerialization();
	
	success &= ipAddress0->serializeIntoParent(this);
	success &= ipAddress1->serializeIntoParent(this);
	success &= ipAddress2->serializeIntoParent(this);
	success &= ipAddress3->serializeIntoParent(this);
	success &= broadcast->serializeIntoParent(this);
	success &= portNumber->serializeIntoParent(this);
	success &= sendingFrequency->serializeIntoParent(this);
	
	success &= networkIpAddress0->serializeIntoParent(this);
	success &= networkIpAddress1->serializeIntoParent(this);
	success &= networkIpAddress2->serializeIntoParent(this);
	success &= networkIpAddress3->serializeIntoParent(this);
	success &= networkMask0->serializeIntoParent(this);
	success &= networkMask1->serializeIntoParent(this);
	success &= networkMask2->serializeIntoParent(this);
	success &= networkMask3->serializeIntoParent(this);
	
	tinyxml2::XMLElement* scriptXML = xmlElement->InsertNewChildElement("Script");
	scriptXML->SetText(script->getScriptText().c_str());
	
	return success;
}


bool ArtNetNode::onDeserialization(){
	bool success = true;
	success &= NetworkDevice::onSerialization();
	success &= ipAddress0->deserializeFromParent(this);
	success &= ipAddress1->deserializeFromParent(this);
	success &= ipAddress2->deserializeFromParent(this);
	success &= ipAddress3->deserializeFromParent(this);
	success &= broadcast->deserializeFromParent(this);
	success &= portNumber->deserializeFromParent(this);
	success &= sendingFrequency->deserializeFromParent(this);
	success &= networkIpAddress0->deserializeFromParent(this);
	success &= networkIpAddress1->deserializeFromParent(this);
	success &= networkIpAddress2->deserializeFromParent(this);
	success &= networkIpAddress3->deserializeFromParent(this);
	success &= networkMask0->deserializeFromParent(this);
	success &= networkMask1->deserializeFromParent(this);
	success &= networkMask2->deserializeFromParent(this);
	success &= networkMask3->deserializeFromParent(this);
	
	if(tinyxml2::XMLElement* scriptXML = xmlElement->FirstChildElement("Script")){
		std::string scriptString = std::string(scriptXML->GetText());
		script->load(scriptString);
		success &= true;
	}
	else success &= false;
	
	return success;
}
