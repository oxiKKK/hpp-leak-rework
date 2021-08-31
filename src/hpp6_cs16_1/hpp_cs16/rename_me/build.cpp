#include "Main.h"

std::vector<BuildInfo> g_BuildInfo;

void CBuild::Add(
	const std::string& a_sBuildName, 
	const std::string& a_sVersion, 
	const std::string& a_sGameName, 
	const std::string& a_sDownloadID, 
	const std::string& a_sConfigFolder, 
	const std::string& a_sProcessName, 
	const std::string& a_sInformation )
{
	BuildInfo buildInfo = 
	{
		a_sBuildName,
		a_sVersion,
		a_sGameName,
		a_sDownloadID,
		a_sConfigFolder,
		a_sProcessName,
		a_sInformation,
	};

	g_BuildInfo.push_back(buildInfo);
}

void CBuild::Add(const BuildInfo& a_BuildInfo)
{
	g_BuildInfo.push_back(a_BuildInfo);
}

CBuild g_Build;