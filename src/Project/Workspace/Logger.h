#pragma once

namespace NewLogger{

void initialize(){
	Logger::init();
}
void terminate(){
	Logger::terminate();
}

bool trace();
bool info();
bool warn();
bool error();
bool critical();

};
