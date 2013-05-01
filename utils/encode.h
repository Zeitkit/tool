#ifndef ENCODE_H
#define ENCODE_H

#include <string>

namespace Utils
{
	std::string JSON_encode(const std::string& input);
	std::string inttostr(unsigned int input);
	std::string inttostr(signed int input);
}

#endif
