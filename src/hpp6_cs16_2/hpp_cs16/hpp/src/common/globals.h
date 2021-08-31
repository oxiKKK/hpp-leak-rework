class CGlobals
{
public:
	CGlobals();

	HMODULE			m_hModule;
	HWND			m_hWnd;
	WNDPROC			m_pWndProc;
	DWORD			m_dwProcessId;
	std::string		m_sSystemDisk;
	std::string		m_sSettingsPath;
	std::string		m_sDebugFile;
	std::string		m_sHWID;
	bool			m_bIsUnloadingLibrary;
	bool			m_bIsGameHooked;
	bool			m_bIsSDL2Hooked;
	bool			m_bIsInGame;
	bool			m_bLicenseChecked;
	bool			m_bIsInThirdPerson;
	bool			m_bSnapshot;
	bool			m_bScreenshot;	
	double			m_dbLastPacketTime;
	double			m_dbLastCheckTime;
	double			m_dbInjectedTime;
	int				m_iGameBuild;
	int				m_iOnlineNum;
	int				m_nNewCommands;
	uintptr_t		m_dwInitPoint;

	// Watches
	float			m_flFrameTime;
	float			m_flGameTime;
	float			m_flRoundTime;

	// Render models
	bool			m_bRenderModels;
	int				m_iRenderType;
	float			m_flRenderColor[4];

	// Local Player Gait Yaw
	float			m_flYaw;
	float			m_flGaitYaw;
	float			m_flGaitMovement;
	float			m_flYawModifier;
	float			m_flYawDiff;
	float			m_flBlendYaw; // Use integer
	Vector			m_vecPreviousGaitOrigin;

	// R_DrawEntitiesOnList
	int*			m_pNumVisibleEntities;
	cl_entity_t**	m_pVisibleEntities;
	uintptr_t		m_dwCurrentEntity;

	bool*			m_bAllowCheats;
};

extern std::unique_ptr<CGlobals> g_pGlobals;