#include <utils/encode.h>

#include <sstream>

using namespace std;

string Utils::json_encode(const string& input)
{
    ostringstream ss;

    for (char c : input)
        switch (c)
        {
            case '\\': ss << "\\\\"; break;
            case '"': ss << "\\\""; break;
            case '/': ss << "\\/"; break;
            case '\b': ss << "\\b"; break;
            case '\f': ss << "\\f"; break;
            case '\n': ss << "\\n"; break;
            case '\r': ss << "\\r"; break;
            case '\t': ss << "\\t"; break;
            default: ss << c; break;
        }

    return ss.str();
}

string Utils::inttostr(unsigned int input)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", input);
	return buf;
}

string Utils::inttostr(signed int input)
{
	char buf[16];
	snprintf(buf, sizeof(buf), "%d", input);
	return buf;
}
