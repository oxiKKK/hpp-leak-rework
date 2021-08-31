class CGlobalsVars;
class CGlobalsVars {
public:
	CGlobalsVars() { RtlSecureZeroMemory(this, sizeof(*this)); }

	HMODULE			m_hModule;
	HWND			m_hWnd;
	DWORD			m_dwProcessID;

	std::string		m_sSettingsPath;
	std::string		m_sDebugFile;
	std::string		m_sOnlineUsers;

	bool			m_bUnloadLibrary;
	bool			m_bSteamOverlay;
	bool			m_bScreenshot;
	bool			m_bSnapshot;
	bool			*m_pbAllowCheats;
	bool			m_bSendCommand;	
	bool			m_bRenderColoredModels;

	int				m_iRenderType;

	float			m_flRenderColor[4];

	short			m_sLastLerpMsec;

	bool			m_bDrawMyVisibleEdicts;
	int				*m_piNumVisibleEntities;
	cl_entity_t		**m_pVisibleEntities;

	bool			m_bLicenseChecked;

	std::string		m_sUUID;
	clock_t			m_LastPacketTime;
	int				m_iSocketServerID;

	double*			m_pGlobalTime;

	float			m_flFrameTime;

	int				m_iGameBuild;
};
extern CGlobalsVars g_Globals;