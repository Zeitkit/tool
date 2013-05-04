#include <utils/parser.h>

#include <stdexcept>

using namespace std;

json_value* Utils::json_parse(char* source, block_allocator* alloc)
{
	char* errorPos;
	char* errorDesc;
	int errorLine;

	json_value* root = json_parse(source, &errorPos, &errorDesc, &errorLine, alloc);

	if (!root)
		throw runtime_error(string("Malformed server response: ") + errorDesc);

	return root;
}

