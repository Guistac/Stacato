#include <pch.h>

#include "PsnServer.h"

#include "Networking/Network.h"

#include "Scripting/Script.h"
#include "Scripting/EnvironnementLibrary.h"
#include "Scripting/LoggingLibrary.h"
#include "Scripting/PsnLibrary.h"

#include <tinyxml2.h>

#include <psn_lib.hpp>

//Operating System Abstraction Layer from libSOEM
//Uselful for precise cross-platform thread sleep
#include <osal.h>

static psn::float3 toFloat3(glm::vec3 in){ return psn::float3(in.x, in.y, in.z); }

void PsnTracker::setPosition(glm::vec3 position){
	psncpptracker->set_pos(toFloat3(position));
}

void PsnTracker::setVelocity(glm::vec3 velocity){
	psncpptracker->set_speed(toFloat3(velocity));
}

void PsnTracker::setAcceleration(glm::vec3 acceleration){
	psncpptracker->set_accel(toFloat3(acceleration));
}

void PsnTracker::setTarget(glm::vec3 targetPosition){
	psncpptracker->set_target_pos(toFloat3(targetPosition));
}

void PsnTracker::setOrigin(glm::vec3 origin){
	psncpptracker->set_ori(toFloat3(origin));
}

void PsnTracker::setStatus(float status){
	psncpptracker->set_status(status);
}

void PsnTracker::setTimestamp(uint64_t timestamp){
	psncpptracker->set_timestamp(timestamp);
}

std::shared_ptr<PsnTracker> PsnServer::createNewTracker(std::string name){
	trackerMutex.lock();
	trackers[trackerCount] = psn::tracker(trackerCount, name);
	auto& newTracker = trackers[trackerCount];
	trackerCount++;
	trackerMutex.unlock();
	return std::make_shared<PsnTracker>(newTracker);
}

void PsnServer::removeAllTrackers(){
	trackerMutex.lock();
	trackers.clear();
	trackerCount = 0;
	trackerMutex.unlock();
}



void PsnServer::onConstruction(){
	NetworkDevice::onConstruction();
	
	setName("PSN Server");
	
	script = std::make_shared<LuaScript>("PSN Server Script");
	script->setLoadLibrairiesCallback([this](lua_State* L){
		Scripting::EnvironnementLibrary::openlib(L);
		auto thisPsnServer = std::dynamic_pointer_cast<PsnServer>(shared_from_this());
		Scripting::PsnLibrary::openlib(L, thisPsnServer);
	});
	std::string defaultScript =
		"\n--Default PSN Server Script"
		"\n"
		"\nfunction setup()"
		"\n"
		"\nend"
		"\n"
		"\n"
		"\nfunction update()"
		"\n"
		"\nend"
		"\n";
	script->load(defaultScript);
	
	infoSendingFrequency = Legato::NumberParameter<double>::createInstance(1.0, "Info Packet sending frequency", "InfoPacketSendingFrequency");
	infoSendingFrequency->setUnit(Units::Frequency::Hertz);
	infoSendingFrequency->allowNegatives(false);
	
	dataSendingFrequency = Legato::NumberParameter<double>::createInstance(60.0, "Data Packet sending frequency", "DataPacketSendingFrequency");
	dataSendingFrequency->setUnit(Units::Frequency::Hertz);
	dataSendingFrequency->allowNegatives(false);

	destinationIp0 = Legato::NumberParameter<uint8_t>::createInstance(236, "PSN Destination IP Octet 0", "DestinationIpOctet0");
	destinationIp1 = Legato::NumberParameter<uint8_t>::createInstance(10, "PSN Destination IP Octet 1", "DestinationIpOctet1");
	destinationIp2 = Legato::NumberParameter<uint8_t>::createInstance(10, "PSN Destination IP Octet 2", "DestinationIpOctet2");
	destinationIp3 = Legato::NumberParameter<uint8_t>::createInstance(10, "PSN Destination IP Octet 3", "DestinationIpOctet3");
	destinationPortNumber = Legato::NumberParameter<uint16_t>::createInstance(56565, "PSN Destination Port Number", "Port");
	
	localIp0 = Legato::NumberParameter<uint8_t>::createInstance(192, "PSN Local IP Octet 0", "LocalIpOctet0");
	localIp1 = Legato::NumberParameter<uint8_t>::createInstance(168, "PSN Local IP Octet 1", "LocalIpOctet1");
	localIp2 = Legato::NumberParameter<uint8_t>::createInstance(0, "PSN Local IP Octet 2", "LocalIpOctet2");
	localIp3 = Legato::NumberParameter<uint8_t>::createInstance(0, "PSN Local IP Octet 3", "LocalIpOctet3");
	
	destinationIp0->addEditCallback([this](){
		uint8_t IP_MSB = destinationIp0->getValue();
		b_addressIsMulticast = IP_MSB >> 4 == 0b1110;
	});
	
}

void PsnServer::connect(){
    udpSocket = Network::getUdpMulticastSocket({localIp0->getValue(), localIp1->getValue(), localIp2->getValue(), localIp3->getValue()},
                                               {destinationIp0->getValue(), destinationIp1->getValue(), destinationIp2->getValue(), destinationIp3->getValue()},
                                               destinationPortNumber->getValue());
	psnEncoder = std::make_shared<psn::psn_encoder>(serverName->getValue());
	if(udpSocket != nullptr && startServer()) b_online = true;
	else b_online = false;
     
}

void PsnServer::disconnect(){
    stopServer();
    b_online = false;
	b_isSending = false;
    udpSocket = nullptr;
}

bool PsnServer::isConnected(){
	return b_online && b_isSending;
}

bool PsnServer::isDetected(){
	return Network::isInitialized();
}

void PsnServer::onConnection(){
}

void PsnServer::onDisconnection(){
}

void PsnServer::readInputs(){
	//incoming messages should be read by and asynchronous asio routine
}

void PsnServer::writeOutputs(){
	//outgoing messages should be sent by their respective timing thread
}



bool PsnServer::startServer(){
	
	assert(false || "Timing source should be reimplemented");
	/*
	removeAllTrackers();
	
	Logger::debug("Starting PSN Server \"{}\"", serverName->value);

	script->compileAndRun();
	script->callFunction("setup");
	serverEnvironnementStartTimeMicroseconds = Environnement::getTime_nanoseconds() / 1000;
	serverProgramStartTimeMicroseconds = Timing::getProgramTime_nanoseconds() / 1000;
	
    
    uint32_t address_u32 = destinationIp0->value << 24 | destinationIp1->value << 16 | destinationIp2->value << 8 | destinationIp3->value;
    asio::ip::address_v4 address = asio::ip::make_address_v4(address_u32);
    asio::ip::udp::endpoint endpoint(address, destinationPortNumber->value);
    
    
	b_serverRunning = true;
	std::thread infoSenderThread([&, this](){
		pthread_setname_np("PSN Info Sender Thread");
		uint32_t sleepTimeMicroseconds = 1000000.0 / infoSendingFrequency->value;
		while(b_serverRunning){
			
            
            
            long long packetTimestamp = (Timing::getProgramTime_nanoseconds() / 1000) - serverProgramStartTimeMicroseconds;
            
            trackerMutex.lock();
            std::list<std::string> info_packets = psnEncoder->encode_info(trackers, packetTimestamp);
            trackerMutex.unlock();
            
            for(auto it = info_packets.begin(); it != info_packets.end(); it++){
                auto packet = *it;
                
                uint8_t* buffer = (uint8_t*)packet.c_str();
                size_t size = packet.length();
                
                try {
                    udpSocket->async_send(asio::buffer(buffer, size), [this](asio::error_code error, size_t byteCount) {
						if (error) b_isSending = false;
						else b_isSending = true;
                    });
                     
                }
                catch (std::exception e) {
                    Logger::error("Failed to start async_send: {}", e.what());
                }
            }
            
            
            
			osal_usleep(sleepTimeMicroseconds);
		}
	});
	infoSenderThread.detach();
    
    
	std::thread dataSenderThread([&, this](){
		pthread_setname_np("PSN Data Sender Thread");
		uint32_t sleepTimeMicroseconds = 1000000.0 / dataSendingFrequency->value;
		while(b_serverRunning){
			script->callFunction("update");
			
            
            
            long long trackerTimestamp = (Environnement::getTime_nanoseconds() / 1000) - serverEnvironnementStartTimeMicroseconds;
            long long packetTimestamp = (Timing::getProgramTime_nanoseconds() / 1000) - serverProgramStartTimeMicroseconds;
            
            script->callFunction("update");
            for(uint16_t i = 0; i < trackerCount; i++) trackers[i].set_timestamp(trackerTimestamp);
            
            trackerMutex.lock();
            std::list<std::string> data_packets = psnEncoder->encode_data(trackers, packetTimestamp);
            trackerMutex.unlock();
            
            for(auto it = data_packets.begin(); it != data_packets.end(); it++){
                auto packet = *it;
                
                uint8_t* buffer = (uint8_t*)packet.c_str();
                size_t size = packet.length();
                
                try {
                    udpSocket->async_send(asio::buffer(buffer, size), [this](asio::error_code error, size_t byteCount) {
						if (error) b_isSending = false;
						else b_isSending = true;
                    });
                }
                catch (std::exception e) {
                    Logger::error("Failed to start async_send: {}", e.what());
                }
            }
            
            
			osal_usleep(sleepTimeMicroseconds);
		}
	});
	dataSenderThread.detach();
	
	Logger::info("Started PSN Server \"{}\"", serverName->value);
	*/
	return true;
}

void PsnServer::stopServer(){
	Logger::info("Stopping PSN Server");
	b_serverRunning = false;
	script->stop();
}



void PsnServer::setDefaultAddressSettings(){
	destinationIp0->overwriteWithHistory(236);
	destinationIp1->overwriteWithHistory(10);
	destinationIp2->overwriteWithHistory(10);
	destinationIp3->overwriteWithHistory(10);
	destinationPortNumber->overwriteWithHistory(56565);
}



bool PsnServer::onSerialization(){
	assert(false && "Cannot save or load this yet");
	using namespace tinyxml2;
	/*
	XMLElement* networkXML = xml->InsertNewChildElement("Network");
	serverName->save(networkXML);
	destinationIp0->save(networkXML);
	destinationIp1->save(networkXML);
	destinationIp2->save(networkXML);
	destinationIp3->save(networkXML);
	destinationPortNumber->save(networkXML);
	localIp0->save(networkXML);
	localIp1->save(networkXML);
	localIp2->save(networkXML);
	localIp3->save(networkXML);
	infoSendingFrequency->save(networkXML);
	dataSendingFrequency->save(networkXML);
	
	XMLElement* scriptXML = xml->InsertNewChildElement("Script");
	scriptXML->SetText(script->getScriptText().c_str());
	*/
	return true;
}


bool PsnServer::onDeserialization(){
	assert(false && "Cannot save or load this yet");
	using namespace tinyxml2;
	/*
	XMLElement* networkXML;
	if(!loadXMLElement("Network", xml, networkXML)) return false;
	if(!serverName->load(networkXML)) return false;
	
	if(!destinationIp0->load(networkXML)) return false;
	if(!destinationIp1->load(networkXML)) return false;
	if(!destinationIp2->load(networkXML)) return false;
	if(!destinationIp3->load(networkXML)) return false;
	if(!destinationPortNumber->load(networkXML)) return false;
    if(!localIp0->load(networkXML)) return false;
    if(!localIp1->load(networkXML)) return false;
    if(!localIp2->load(networkXML)) return false;
    if(!localIp3->load(networkXML)) return false;
	 
	if(!infoSendingFrequency->load(networkXML)) return false;
	if(!dataSendingFrequency->load(networkXML)) return false;
	destinationIp0->onEdit();
	
	XMLElement* scriptXML;
	if(!loadXMLElement("Script", xml, scriptXML)) return false;
	std::string scriptString = std::string(scriptXML->GetText());
	script->load(scriptString);
	*/
	return true;
}
