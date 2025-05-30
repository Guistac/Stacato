#include <pch.h>
#include "ConsoleHandler.h"

#include "Serial.h"
#include "Console.h"
#include "ConsoleMapping.h"

#include "Consoles/StacatoCompact.h"
#include "Consoles/ButtonBoard.h"
#include "Consoles/ConsoleStarmania.h"
#include "Consoles/StacatoV2.h"

namespace ConsoleHandler{

	std::string consolePortName = "Stacato";
	std::shared_ptr<Console> connectedConsole = nullptr;

	std::thread consoleHandler;
	bool b_handlerRunning = false;

	std::shared_ptr<Console> getConnectedConsole(){
		return connectedConsole;
	}

	void update(){
		//pthread_setname_np("Console Handler Thread");
		while(b_handlerRunning){
			if(connectedConsole){
				if(connectedConsole->wasDisconnected()) {
					Console::terminate(connectedConsole);
					connectedConsole = nullptr;
				}
			}else{
				auto serialPort = findSerialPort(consolePortName);
				if(serialPort) connectedConsole = Console::initialize(serialPort);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(1000));
		}
	}

	void initialize(){
		if(b_handlerRunning) return;
		b_handlerRunning = true;
		consoleHandler = std::thread(update);
	}

	void terminate(){
		if(!b_handlerRunning) return;
		b_handlerRunning = false;
		consoleHandler.join();
	}

	std::vector<std::shared_ptr<ConsoleMapping>> mappingDictionnary = {
		std::make_shared<ConsoleStarmania>(),
		std::make_shared<StacatoCompact>(),
		std::make_shared<StacatoV2>()
	};

	void applyMapping(std::shared_ptr<Console> console){
		for(auto mapping : mappingDictionnary){
			if(mapping->matchesConsole(console)){
				console->setMapping(mapping);
                return;
			}
		}
	}

};
