#include <pch.h>

#include "PsnServer.h"

#include "Networking/Network.h"

#include <tinyxml2.h>

void PsnServer::initialize(){}

void PsnServer::connect(){
	onConnection();
}

void PsnServer::disconnect(){
	onDisconnection();
}

bool PsnServer::isConnected(){
	return false;
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

bool PsnServer::load(tinyxml2::XMLElement* xml){
	return true;
}
bool PsnServer::save(tinyxml2::XMLElement* xml){
	return true;
}
