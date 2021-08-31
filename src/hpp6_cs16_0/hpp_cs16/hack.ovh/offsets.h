struct Module {
	DWORD base;
	DWORD size;
	DWORD end;
};

class COffsets;
class COffsets {
public:
	Module hw, client, gameui, vgui2, gor;

	DWORD m_dwSendCommandPointer1, m_dwSendCommandPointer2;
	DWORD m_dwCloudOffsets[2];

	bool FindHardware(void);
	bool FindOffsets(void);
	bool FindModuleByName(const char* moduleName, Module* module);

	void Error(const char *fmt, ...);

	DWORD FindPattern(PCHAR pszPattern, DWORD dwStart, DWORD dwEnd, DWORD dwOffset = 0);
	DWORD FindPattern(PCHAR pszPattern, PCHAR pszMask, DWORD dwStart, DWORD dwEnd, DWORD dwOffset = 0);
	DWORD Absolute(DWORD dwAddress);
	DWORD FindReference(DWORD dwStart, DWORD dwEnd, DWORD dwAddress);
	DWORD FindPush(PCHAR pszMessage, DWORD dwStart, DWORD dwEnd, DWORD dwOffset = 0);
	DWORD FarProc(const DWORD dwAddress, DWORD LB, DWORD HB);

	bool FindSendCommand(void);
	DWORD PreS_DynamicSound(void);
	DWORD AllowCheats(void);
	DWORD FindExtraPlayerInfo(void);
	DWORD FindSteamOverlay(void);
	PVOID FindEngineMessages(void);
	PVOID FindClient(void);
	PVOID FindEngine(void);
	PVOID FindStudio(void);
	PVOID FindStudioRenderModel(void);
	PVOID FindPlayerMove(void);
	PVOID FindClientUserMsgs(void);
	PVOID FindClientState(void);
	PVOID FindClientStatic(void);
	DWORD R_DrawEntitiesOnList();
};
extern COffsets g_Offsets;
