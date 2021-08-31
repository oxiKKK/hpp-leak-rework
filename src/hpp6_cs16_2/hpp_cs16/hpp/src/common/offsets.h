using PreS_DynamicSound_t = void(__cdecl*)(int, DWORD, char*, float*, float, float, int, int);
using CL_DriftInterpolationAmount_t = int(__cdecl*)(int);
using CL_RecordHUDCommand_t = void(__cdecl*)(const char*);
using Host_FilterTime_t = int(__cdecl*)(float);
using SCR_UpdateScreen_t = void(__cdecl*)();
using R_DrawEntitiesOnList_t = void(__cdecl*)();
using InitiateGameConnection_t = int(__cdecl*)(void* pAuthBlob, int cbMaxAuthBlob, CSteamID steamIDGameServer, uint32 unIPServer, uint16 usPortServer, bool bSecure);
using CL_RunUsercmd_t = void(__cdecl*)(local_state_t* from, local_state_t* to, usercmd_t* u, qboolean runfuncs, double* time, unsigned int random_seed);
using NET_SendPacket_t = int(__cdecl*)(netsrc_t sock, size_t length, const void* data, netadr_t to);
using IN_MouseMove_t = void(__cdecl*)(float, usercmd_t*);

extern PreS_DynamicSound_t g_pPreS_DynamicSound;
extern CL_DriftInterpolationAmount_t g_pCL_DriftInterpolationAmount;
extern CL_RecordHUDCommand_t g_pCL_RecordHUDCommand;
extern R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList;
extern InitiateGameConnection_t g_pInitiateGameConnection;
extern NET_SendPacket_t g_pNET_SendPacket;
extern CL_RunUsercmd_t g_pCL_RunUsercmd;
extern Host_FilterTime_t g_pHost_FilterTime;
extern SCR_UpdateScreen_t g_pSCR_UpdateScreen;
extern PClientUserMsg g_pClientUserMsgs;
extern IN_MouseMove_t g_pIN_MouseMove;

class COffsets
{
private:
	enum ModulesList
	{
		HW,					 // "hw.dll"
		CLIENT,				 // "client.dll"
		GAMEUI,				 // "gameui.dll"
		VGUI2,				 // "vgui2.dll"
		GAMEOVERLAYRENDERER, // "gameoverlayrenderer.dll"
		STEAMCLIENT,		 // "steamclient.dll"
		MODULES_MAX_COUNT
	};

	struct ModuleInfo
	{
		HMODULE handle;
		uintptr_t base;
		uintptr_t end;
		size_t size;
	} Module[MODULES_MAX_COUNT];

public:
	bool			FindModules();
	bool			FindInterfaces();
	bool			FindGameOffsets();

private:
	bool			FindModuleByName(const char* name, ModuleInfo* module);
	bool			AddressNotInSpace(uintptr_t ptr, uintptr_t low, uintptr_t high);

	PVOID			Client();
	PVOID			Studio();
	PVOID			Engine();
	PVOID			StudioAPI();
	PVOID			PlayerMove();
	PVOID			ClientStatic();
	PVOID			ClientState();
	PVOID			EngineMsgs();
	PVOID			ClientUserMsgs();
	PVOID			PreS_DynamicSound();
	PVOID			CL_DriftInterpolationAmount();
	PVOID			CL_RecordHUDCommand();
	PVOID			Host_FilterTime();
	PVOID			SCR_UpdateScreen();
	PVOID			PlayerExtraInfo();
	PVOID			StudioModelRenderer();
	PVOID			InitPoint();
	PVOID			R_DrawEntitiesOnList();
	PVOID			InitiateGameConnection();
	PVOID			CL_RunUsercmd();
	PVOID			NET_SendPacket();
	PVOID			IN_MouseMove();
	PVOID			AllowCheats();

	int				GetGameBuild();

	uintptr_t		FindDownPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindDownPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindUpPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindUpPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindPush(PCHAR message, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		Absolute(uintptr_t ptr);

	bool			m_bModulesFound;
	bool			m_bInterfacesFound;
};