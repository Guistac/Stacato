#pragma once

namespace Transport{

	std::string microsecondsToTimecodeString(long long int microseconds);
	
	long long int timecodeStringToMicroseconds(const char* buffer);

};
