#ifndef TIME_H
#define TIME_H

#include <string>
#include <ctime>

namespace Time
{
	std::string UnixtimeToLocaltime(time_t time);
	time_t LocaltimeToUnixtime(const std::string& time);
}

#endif
