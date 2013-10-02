#include <utils/time.h>

using namespace std;

string Time::UnixtimeToLocaltime(time_t time)
{
	auto tm_data = localtime(&time);

	char dest[32];
	strftime(dest, sizeof(dest), "%Y-%m-%d-%H:%M:%S", tm_data);

	return dest;
}

time_t Time::LocaltimeToUnixtime(const std::string& time)
{
	tm tm_data;
	sscanf(time.c_str(), "%04d-%02d-%02d-%02d:%02d:%02d", &tm_data.tm_year, &tm_data.tm_mon, &tm_data.tm_mday, &tm_data.tm_hour, &tm_data.tm_min, &tm_data.tm_sec);

	tm_data.tm_year -= 1900;
	--tm_data.tm_mon;
	tm_data.tm_isdst = -1;

	return mktime(&tm_data);
}
