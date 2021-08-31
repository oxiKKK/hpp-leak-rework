#include "framework.h"

std::unique_ptr<CSandbox> g_pSandbox;

CSandbox::CSandbox()
{
	// run code one time when you connected to server
}

CSandbox::~CSandbox()
{
	// run code one time when you disconnected from server
}

std::string CSandbox::GetCvar(std::string name)
{
	g_pConsole->DPrintf(V("> %s: get %s.\n"), V(__FUNCTION__), name.c_str());

	if (!cvars[name].empty())
		return cvars[name];
	
	return std::to_string(g_Engine.pfnGetCvarFloat(name.c_str()));
}

void CSandbox::SetupCvar(std::string name, std::string value)
{
	cvars[name] = value;

	g_pConsole->DPrintf(V("> %s: setup %s => %s\n"), V(__FUNCTION__), name.c_str(), value.c_str());
}