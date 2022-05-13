#include <pch.h>
#include "Playback.h"

namespace Playback::Transport{

	long long int weekInMicroseconds = 604'800'000'000;
	long long int dayInMicroseconds = 86'400'000'000;
	long long int hourInMicroseconds = 3'600'000'000;
	long long int minuteInMicroseconds = 60'000'000;
	long long int secondInMicroseconds = 1'000'000;

	std::string microsecondsToTimecodeString(long long int microseconds){
		static char buffer[256];
		int start = 0;
		
		bool b_negative = microseconds < 0;
		microseconds = std::abs(microseconds);
		
		int weeks = microseconds / weekInMicroseconds;
		microseconds -= weeks * weekInMicroseconds;
		
		int days = microseconds / dayInMicroseconds;
		microseconds -= days * dayInMicroseconds;
		
		int hours = microseconds / hourInMicroseconds;
		microseconds -= hours * hourInMicroseconds;
		
		int minutes = microseconds / minuteInMicroseconds;
		microseconds -= minutes * minuteInMicroseconds;
		
		double seconds = double(microseconds) / 1000000.0;
		
		start += sprintf(buffer + start, b_negative ? "-" : "+");
		start += sprintf(buffer + start, "%.2i:", hours);
		start += sprintf(buffer + start, "%.2i:", minutes);
		start += sprintf(buffer + start, "%04.1f", seconds);
		
		return std::string(buffer);
	}



	long long int timecodeStringToMicroseconds(const char* buffer){
		
		std::vector<std::deque<char>> timeChars(3);

		enum timeUnit {
			seconds = 0,
			minutes = 1,
			hours = 2
		};

		int timeUnit = seconds;
		int charsInCurrentUnit = 0;
		bool unitForced = false;

		size_t stringLength = strlen(buffer);
		for (int i = (int)stringLength - 1; i >= 0; i--) {
			char c = buffer[i];
			if (c >= 48 && c <= 57) {
				timeChars[timeUnit].push_front(c);
				charsInCurrentUnit++;
			} //number
			else if (c == '.' || c == ',') {
				timeChars[timeUnit].push_front(c);
				charsInCurrentUnit = 0;
			} //point
			else if (c == ':' || c == '/') {
				if (charsInCurrentUnit != 0) {
					timeUnit++;
					charsInCurrentUnit = 0;
				}
			} //double column
			else if (c == 'h' || c == 'H') {
				if (timeChars[minutes].empty()) {
					timeChars[minutes] = timeChars[seconds];
					timeChars[seconds].clear();
				}
				charsInCurrentUnit = 0;
				timeUnit = hours;
				unitForced = true;
			}
			else if (c == 'm' || c == 'M') {
				charsInCurrentUnit = 0;
				timeUnit = minutes;
				unitForced = true;
			}
			else if (c == 's' || c == 'S') {
				charsInCurrentUnit = 0;
				timeUnit = seconds;
				unitForced = true;
			}
			if (!unitForced && charsInCurrentUnit == 2) {
				charsInCurrentUnit = 0;
				timeUnit++;
			}
			if (timeUnit > hours) {
				break;
			}
		}

		std::vector<char> hh;
		std::vector<char> mm;
		std::vector<char> ss;

		if (!timeChars[hours].empty()) for (char c : timeChars[hours]) hh.push_back(c);
		else hh.push_back('0');
		hh.push_back(0);
		if (!timeChars[minutes].empty()) for (char c : timeChars[minutes]) mm.push_back(c);
		else mm.push_back('0');
		mm.push_back(0);
		if (!timeChars[seconds].empty()) for (char c : timeChars[seconds]) ss.push_back(c);
		else ss.push_back('0');
		ss.push_back(0);

		int h;
		int m;
		float s;

		std::stringstream hstream(hh.data());
		std::stringstream mstream(mm.data());
		std::stringstream sstream(ss.data());

		hstream >> h;
		mstream >> m;
		sstream >> s;
		
		long long int microseconds = 0;
		microseconds += h * hourInMicroseconds;
		microseconds += m * minuteInMicroseconds;
		microseconds += s * secondInMicroseconds;
		
		return microseconds;
	}

	std::string secondsToTimecodeString(double seconds){
		return microsecondsToTimecodeString(seconds * 1000000);
	}

	double timecodeStringToSeconds(const char* buffer){
		return timecodeStringToMicroseconds(buffer) / 1000000.0;
	}


};
