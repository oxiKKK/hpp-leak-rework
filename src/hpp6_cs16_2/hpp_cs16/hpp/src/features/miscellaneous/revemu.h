typedef struct revEmuTicket_s {
	uint32_t version;
	uint32_t highPartAuthID;
	uint32_t signature;
	uint32_t secondSignature;
	uint32_t authID;
	uint32_t thirdSignature;
	uint8_t  hash[128];
} revEmuTicket_t;

namespace RevEmu
{
	extern uint8_t HashSymbolTable[36];
	extern uint32_t Hash(const char* str);
}