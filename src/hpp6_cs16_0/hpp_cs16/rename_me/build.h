class CBuild;

struct BuildInfo 
{
	std::string m_sBuildName;
	std::string m_sVersion;
	std::string m_sGameName;
	std::string m_sUrl;
	std::string m_sConfigFolder;
	std::string m_sProcessName;
	std::string m_sInformation;
};

extern std::vector<BuildInfo> g_BuildInfo;

class CBuild 
{
public:
	void Add(
		const std::string& a_sBuildName, 
		const std::string& a_sVersion, 
		const std::string& a_sGameName, 
		const std::string& a_sUrl,
		const std::string& a_sConfigFolder, 
		const std::string& a_sProcessName, 
		const std::string& a_sInformation);

	void Add(const BuildInfo& a_BuildInfo);
};

extern CBuild g_Build;