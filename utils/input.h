#ifndef INPUT_H
#define INPUT_H

#include <string>
#include <fstream>

namespace Utils
{
	void SetStdinEcho(bool enable);
	std::string get_file_contents(std::ifstream& input);
}

#endif
