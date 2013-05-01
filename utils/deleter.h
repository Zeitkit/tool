#ifndef DELETER_H
#define DELETER_H

#include <memory>

namespace Utils
{
	template<typename T>
	struct free_delete
	{
		void operator()(T* data) { std::free(data); }
	};
}

#endif
