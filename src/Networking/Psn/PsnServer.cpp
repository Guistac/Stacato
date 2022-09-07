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



void PsnServer::initialize(){
	
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
	
	ipAddress0->setEditCallback([this](std::shared_ptr<Parameter>){
		uint8_t IP_MSB = ipAddress0->value;
		b_addressIsMulticast = IP_MSB >> 4 == 0b1110;
	});
	
}

void PsnServer::connect(){
    /*
    udpSocket = Network::getUdpMulticastSocket({networkIpAddress0->value, networkIpAddress1->value, networkIpAddress2->value, networkIpAddress3->value},
                                               {ipAddress0->value, ipAddress1->value, ipAddress2->value, ipAddress3->value},
                                               portNumber->value);
	psnEncoder = std::make_shared<psn::psn_encoder>(serverName->value);
	if(udpSocket != nullptr && startServer()) b_online = true;
	else b_online = false;
     */
}

void PsnServer::disconnect(){
    stopServer();
    b_online = false;
    udpSocket = nullptr;
}

bool PsnServer::isConnected(){
	return b_online;
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
	
	removeAllTrackers();
	
	Logger::debug("Starting PSN Server \"{}\"", serverName->value);

	script->compileAndRun();
	script->callFunction("setup");
	serverEnvironnementStartTimeMicroseconds = Environnement::getTime_nanoseconds() / 1000;
	serverProgramStartTimeMicroseconds = Timing::getProgramTime_nanoseconds() / 1000;
	
    
    uint32_t address_u32 = ipAddress0->value << 24 | ipAddress1->value << 16 | ipAddress2->value << 8 | ipAddress3->value;
    asio::ip::address_v4 address = asio::ip::make_address_v4(address_u32);
    asio::ip::udp::endpoint endpoint(address, portNumber->value);
    
    
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
                    /*
                    udpSocket->async_send_to(asio::buffer(buffer, size), endpoint, [](asio::error_code error, size_t byteCount){
                        if (error) Logger::debug("Failed to send PSN Info Message: {}", error.message());
                    });
                     */
                    
                    udpSocket->async_send(asio::buffer(buffer, size), [](asio::error_code error, size_t byteCount) {
                        if (error) Logger::debug("Failed to send PSN Info Message: {}", error.message());
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
                    /*
                    udpSocket->async_send_to(asio::buffer(buffer, size), endpoint, [](asio::error_code error, size_t byteCount){
                        if (error) Logger::debug("Failed to send PSN Data Message: {}", error.message());
                    });
                    */
                    udpSocket->async_send(asio::buffer(buffer, size), [](asio::error_code error, size_t byteCount) {
                        if (error) Logger::debug("Failed to send PSN Data Message: {}", error.message());
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
	
	return true;
}

void PsnServer::stopServer(){
	Logger::info("Stopping PSN Server");
	b_serverRunning = false;
	script->stop();
}

void PsnServer::sendInfo(){
	
	long long packetTimestamp = (Timing::getProgramTime_nanoseconds() / 1000) - serverProgramStartTimeMicroseconds;
	
	trackerMutex.lock();
	std::list<std::string> info_packets = psnEncoder->encode_info(trackers, packetTimestamp);
	trackerMutex.unlock();
	
	for(auto it = info_packets.begin(); it != info_packets.end(); it++){
		auto packet = *it;
		
		uint8_t* buffer = (uint8_t*)packet.c_str();
		size_t size = packet.length();
		
		try {
			udpSocket->async_send(asio::buffer(buffer, size), [](asio::error_code error, size_t byteCount) {
				if (error) Logger::debug("Failed to send PSN Info Message: {}", error.message());
			});
		}
		catch (std::exception e) {
			Logger::error("Failed to start async_send: {}", e.what());
		}
	}
}

void PsnServer::sendData(){

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
			udpSocket->async_send(asio::buffer(buffer, size), [](asio::error_code error, size_t byteCount) {
				if (error) Logger::debug("Failed to send PSN Data Message: {}", error.message());
			});
		}
		catch (std::exception e) {
			Logger::error("Failed to start async_send: {}", e.what());
		}
	}
}



void PsnServer::setDefaultAddressSettings(){
	ipAddress0->overwriteWithHistory(236);
	ipAddress1->overwriteWithHistory(10);
	ipAddress2->overwriteWithHistory(10);
	ipAddress3->overwriteWithHistory(10);
	portNumber->overwriteWithHistory(56565);
}



bool PsnServer::save(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* networkXML = xml->InsertNewChildElement("Network");
	serverName->save(networkXML);
	ipAddress0->save(networkXML);
	ipAddress1->save(networkXML);
	ipAddress2->save(networkXML);
	ipAddress3->save(networkXML);
	portNumber->save(networkXML);
	infoSendingFrequency->save(networkXML);
	dataSendingFrequency->save(networkXML);
	
	networkIpAddress0->save(networkXML);
	networkIpAddress1->save(networkXML);
	networkIpAddress2->save(networkXML);
	networkIpAddress3->save(networkXML);
	
	networkMask0->save(networkXML);
	networkMask1->save(networkXML);
	networkMask2->save(networkXML);
	networkMask3->save(networkXML);
	
	XMLElement* scriptXML = xml->InsertNewChildElement("Script");
	scriptXML->SetText(script->getScriptText().c_str());
	
	return true;
}


bool PsnServer::load(tinyxml2::XMLElement* xml){
	using namespace tinyxml2;
	
	XMLElement* networkXML;
	if(!loadXMLElement("Network", xml, networkXML)) return false;
	if(!serverName->load(networkXML)) return false;
	if(!ipAddress0->load(networkXML)) return false;
	if(!ipAddress1->load(networkXML)) return false;
	if(!ipAddress2->load(networkXML)) return false;
	if(!ipAddress3->load(networkXML)) return false;
	if(!portNumber->load(networkXML)) return false;
    if(!networkIpAddress0->load(networkXML)) return false;
    if(!networkIpAddress1->load(networkXML)) return false;
    if(!networkIpAddress2->load(networkXML)) return false;
    if(!networkIpAddress3->load(networkXML)) return false;
    if(!networkMask0->load(networkXML)) return false;
    if(!networkMask1->load(networkXML)) return false;
    if(!networkMask2->load(networkXML)) return false;
    if(!networkMask3->load(networkXML)) return false;
	if(!infoSendingFrequency->load(networkXML)) return false;
	if(!dataSendingFrequency->load(networkXML)) return false;
	ipAddress0->onEdit();
	
	XMLElement* scriptXML;
	if(!loadXMLElement("Script", xml, scriptXML)) return false;
	std::string scriptString = std::string(scriptXML->GetText());
	script->load(scriptString);
	
	return true;
}
