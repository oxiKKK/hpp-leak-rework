class CGlobalsVars;
class CGlobalsVars {
public:
    HMODULE m_hModule					= nullptr;

	DWORD	m_dwProcessID				= 0;

    bool	send_command				= true;
    bool	interpolate					= false; //true if client interpolation active

    float	backtrack					= 0;

    std::string	m_sSettingsPath			= {};
	std::string m_sDebugFile			= {};

    bool	m_bRenderColoredModels		= false;
	int		m_iRenderType				= 0;
	float	m_flRenderColor[3]			= {};

	bool	m_bUnloading				= false;

	bool	*allow_cheats				= nullptr;

	bool	m_bScreenshot				= false;
	bool	m_bSnapshot					= false;

	int		*cl_numvisedicts			= nullptr;
	cl_entity_t **cl_visedicts			= nullptr;
};
extern CGlobalsVars g_Globals;