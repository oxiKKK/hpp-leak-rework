extern bool bInitializedBuilds;

void StartPoint();
void HardwareID();
void General();

extern std::vector<std::string> g_BuildsList;

DWORD FindProcessId(const std::string& sProcessName) noexcept;

struct Globals
{
	int		m_iCurrentBuild;
	int		m_iSavedCurrentBuild;
	DWORD	m_dwCurrentTime;
	DWORD	m_dwProcessID;
	DWORD	m_dwDownloadedBytes;
	bool	m_bIsDownloadThreadStart;
	bool	m_bIsInjectingThreadStart;
	std::string m_sResponse;
};

extern Globals g;