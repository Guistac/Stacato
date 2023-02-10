#pragma once

namespace NewLogger{

void initialize(){}
void terminate(){}

bool trace();
bool info();
bool warn();
bool error();
bool critical();

};
