int GenerateRevEmu(void *pDest, int nSteamID);

namespace RevSpoofer
{
	bool Spoof(char *pszDest, int uSID);
	unsigned int Hash(char *pszString);
}