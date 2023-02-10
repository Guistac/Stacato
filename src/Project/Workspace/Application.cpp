#include <pch.h>

#include "Application.h"
#include "Logger.h"

namespace Application{
	
	std::function<bool()> userInitializationFunction;
	std::function<bool()> userTerminationFunction;
	
	bool b_running = false;
	
	void setInitializationFunction(std::function<bool()> fn){ userInitializationFunction = fn; }
	
	void setTerminationFunction(std::function<bool()> fn){ userTerminationFunction = fn; }
	
	void run(){
		
		NewLogger::initialize();
		NewGui::initialize();
		
		userInitializationFunction();
		
		b_running = true;
		while(b_running){
			NewGui::drawFrame();
		}
		
		userTerminationFunction();
		
		NewGui::terminate();
		Logger::terminate();
		
	}
	
	void requestQuit(){
		//if the current project can close, quit immediately
		//else should send some signal to gui library to make qui popup come up
	}
	
	void quitImmediately(){ b_running = false; }
	
};
