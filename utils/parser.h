#ifndef PARSER_H
#define PARSER_H

#include <json.h>

namespace Utils
{
	json_value* json_parse(char* source, block_allocator* alloc);
}

#endif
