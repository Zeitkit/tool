#ifndef CHECKSUM_H
#define CHECKSUM_H

namespace Utils
{
	bool crc32file(const char* name, unsigned int* crc);
}

#endif
