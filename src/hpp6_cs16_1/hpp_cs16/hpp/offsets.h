class COffsets;
class COffsets {
public:
	Module			hw, client, gameui, vgui2, gameoverlayrenderer, steamclient;

	DWORD			m_dwCloudOffsets[2],
		m_dwpSpeed,
		m_dwInitPoint,
		m_dwInterpolationTime;

	unsigned char	m_puszPatchInterpolationTimeBackup[5];

	bool			FindModules(void);
	bool			FindInterfaces(void);
	bool			FindSteamInterfaces(CreateInterfaceFn steamclient);
	bool			FindOffsets(void);
	bool			FindModuleByName(const char* name, Module* module);
	bool			AddressNotInSpace(uintptr_t ptr, uintptr_t low, uintptr_t high);
	bool			SendCommand(void);
	bool			PatchInterpolationTime(void);
	bool			GlobalTime(void);

	int				GetGameBuild();

	PVOID			ClientStatic(void);
	PVOID			ClientUserMsgs(void);
	PVOID			StudioRenderModel(void);
	PVOID			Client(void);
	PVOID			Engine(void);
	PVOID			Studio(void);
	PVOID			PlayerMove(void);
	PVOID			ClientState(void);

	uintptr_t		FindSteamOverlay(void);
	uintptr_t		EngineMsgs(void);
	uintptr_t		Particles(void);
	uintptr_t		FindSpeed(void);
	uintptr_t		PreS_DynamicSound(void);
	uintptr_t		AllowCheats(void);
	uintptr_t		ExtraPlayerInfo(void);
	uintptr_t		FireBullets(void);
	uintptr_t		FindInitPoint(void);
	uintptr_t		KeyBindings(void);
	uintptr_t		R_DrawEntitiesOnList(void);
	uintptr_t		CL_RecordHUDCommand(void);
	uintptr_t		InitiateGameConnection(void);
	uintptr_t		Absolute(uintptr_t ptr);
	uintptr_t		FindReference(uintptr_t start, uintptr_t end, uintptr_t ptr);
	uintptr_t		FindDownPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindDownPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindUpPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindUpPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset = 0);
	uintptr_t		FindPush(PCHAR message, uintptr_t start, uintptr_t end, uintptr_t offset = 0);

	void			Error(const char *fmt, ...);
	void			CL_ConnectionlessPacketRedirectCall(void);
	void			CL_ReadDemoMessageCall(void);
};
extern COffsets g_Offsets;
