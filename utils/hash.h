#ifndef HASH_H
#define HASH_H

#include <memory>

namespace Utils
{
	template<std::size_t N>
	constexpr unsigned int hash(const char(&str)[N], std::size_t I = N) {
		return (I == 1 ? ((2166136261u ^ str[0]) * 16777619u) : ((hash(str, I - 1) ^ str[I - 1]) * 16777619u));
	}

	unsigned int hash(const char* str, std::size_t I) {
		return (I == 1 ? ((2166136261u ^ str[0]) * 16777619u) : ((hash(str, I - 1) ^ str[I - 1]) * 16777619u));
	}

}
#endif
