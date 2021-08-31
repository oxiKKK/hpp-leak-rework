#include "framework.h"

namespace RevEmu
{
	uint8_t HashSymbolTable[36] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z' };

	uint32_t Hash(const char* str)
	{
		uint32_t hash = 0x4E67C6A7;

		for (const char* pch = str; *pch != '\0'; pch++)
			hash ^= (hash >> 2) + (hash << 5) + *pch;

		return hash;
	}
}