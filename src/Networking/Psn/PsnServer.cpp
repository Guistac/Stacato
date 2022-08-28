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

void PsnServer::initialize(){
	
	script = std::make_shared<LuaScript>("PSN Server Script");
	script->setLoadLibrairiesCallback([](lua_State* L){
		Scripting::EnvironnementLibrary::openlib(L);
		Scripting::PsnLibrary::openlib(L);
		//push some sort of server object onto the lua stack
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
	udpSocket = Network::getUdpSocket(0, {236,10,10,10}, 56565);
	psnEncoder = std::make_shared<psn::psn_encoder>(serverName->value);
	if(udpSocket != nullptr && startServer()) b_online = true;
	else b_online = false;
}

void PsnServer::disconnect(){
	stopServer();
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
	
	script->callFunction("Setup");
	
	trackers[0] = psn::tracker(0, "testTracker");
	trackers[1] = psn::tracker(0, "tracker 1");
	trackers[2] = psn::tracker(0, "Costière Cour");
	trackers[3] = psn::tracker(0, "Etat Flip");

	Logger::info("Startin PSN Server");

	b_serverRunning = true;
	std::thread infoSenderThread([this](){
		pthread_setname_np("PSN Info Sender Thread");
		uint32_t sleepTimeMicroseconds = 1000000.0 / infoSendingFrequency->value;
		while(b_serverRunning){
			sendInfo();
			osal_usleep(sleepTimeMicroseconds);
		}
	});
	infoSenderThread.detach();
	std::thread dataSenderThread([this](){
		pthread_setname_np("PSN Data Sender Thread");
		uint32_t sleepTimeMicroseconds = 1000000.0 / dataSendingFrequency->value;
		while(b_serverRunning){
			script->callFunction("update");
			sendData();
			osal_usleep(sleepTimeMicroseconds);
		}
	});
	dataSenderThread.detach();
	
	return true;
}

void PsnServer::stopServer(){
	Logger::info("Stopping PSN Server");
	b_serverRunning = false;
}

void PsnServer::sendInfo(){
		
	std::list<std::string> info_packets = psnEncoder->encode_info(trackers, 0);
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

	trackers[0].set_pos(psn::float3(0.0, 1.0, 2.0));
	trackers[1].set_pos(psn::float3(0.0, 1.0, 2.0));
	trackers[2].set_pos(psn::float3(0.0, 1.0, 2.0));
	trackers[3].set_pos(psn::float3(0.0, 1.0, 2.0));
	
	std::list<std::string> data_packets = psnEncoder->encode_data(trackers, 0);
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
	if(!infoSendingFrequency->load(networkXML)) return false;
	if(!dataSendingFrequency->load(networkXML)) return false;
	ipAddress0->onEdit();
	
	XMLElement* scriptXML;
	if(!loadXMLElement("Script", xml, scriptXML)) return false;
	std::string scriptString = std::string(scriptXML->GetText());
	script->load(scriptString);
	
	return true;
}
