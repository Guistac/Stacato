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



void ArtNetNode::initialize(){
	
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
	
	ArtConfig config;
}

void ArtNetNode::connect(){
	//we are sending artnet packets over udp (broadcast or not)
    /*
    uint8_t ip0 = networkIpAddress0->value;
    
	if(broadcast->value) {
		udpSocket = Network::getUdpBroadcastSocket();
	}
	else udpSocket = Network::getUdpSocket(0, {ipAddress0->value, ipAddress1->value, ipAddress2->value, ipAddress3->value}, portNumber->value);
	if(udpSocket == nullptr){
		b_running = false;
		return;
	}
	start();
     */
}


void ArtNetNode::start(){
	b_running = true;
	std::thread dmxThread([this](){
		
		long long waitTimeMicros = 1000000.0 / sendingFrequency->value;
		removeAllUniverses();
		
		script->compileAndRun();
		script->callFunction("setup");
		
        uint32_t address_u32 = networkIpAddress0->value << 24 | networkIpAddress1->value << 16 | networkIpAddress2->value << 8 | networkIpAddress3->value;
        uint32_t mask_u32 = networkMask0->value << 24 | networkMask1->value << 16 | networkMask2->value << 8 | networkMask3->value;
        asio::ip::address_v4 address = asio::ip::make_address_v4(address_u32);
        asio::ip::address_v4 mask = asio::ip::make_address_v4(mask_u32);
        asio::ip::network_v4 network(address, mask);
        asio::ip::udp::endpoint broadcastEndpoint(network.broadcast(), portNumber->value);

		bool b_broadcast = broadcast->value;
		
		while(b_running){
			
			script->callFunction("update");
			
			for(auto universe : universes){
				uint8_t* buffer = universe->getBuffer();
				size_t bufferSize = universe->getBufferSize();
				try {
					
					if(b_broadcast){
						udpSocket->async_send_to(asio::buffer(buffer, bufferSize), broadcastEndpoint, [](asio::error_code error, size_t byteCount){
							if(error) Logger::debug("failed to send Artnet boradcast dmw message: {}", error.message());
						});
					}else{
						udpSocket->async_send(asio::buffer(buffer, bufferSize), [](asio::error_code error, size_t byteCount) {
							if (error) Logger::debug("Failed to send ArtNet DMX Message: {}", error.message());
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
	broadcast->save(settingsXML);
	portNumber->save(settingsXML);
	sendingFrequency->save(settingsXML);
    
    networkIpAddress0->save(settingsXML);
    networkIpAddress1->save(settingsXML);
    networkIpAddress2->save(settingsXML);
    networkIpAddress3->save(settingsXML);
    networkMask0->save(settingsXML);
    networkMask1->save(settingsXML);
    networkMask2->save(settingsXML);
    networkMask3->save(settingsXML);
    
    
	
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
	if(!broadcast->load(settingsXML)) return false;
	if(!sendingFrequency->load(settingsXML)) return false;
    if(!networkIpAddress0->load(settingsXML)) return false;
    if(!networkIpAddress1->load(settingsXML)) return false;
    if(!networkIpAddress2->load(settingsXML)) return false;
    if(!networkIpAddress3->load(settingsXML)) return false;
    if(!networkMask0->load(settingsXML)) return false;
    if(!networkMask1->load(settingsXML)) return false;
    if(!networkMask2->load(settingsXML)) return false;
    if(!networkMask3->load(settingsXML)) return false;
	
	XMLElement* scriptXML;
	if(!loadXMLElement("Script", xml, scriptXML)) return false;
	std::string scriptString = std::string(scriptXML->GetText());
	script->load(scriptString);
	
	return true;
}
