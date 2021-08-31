#include "framework.h"

std::unique_ptr<CGlobals> g_pGlobals;

std::string GetSettingsPath()
{
	char* pbuf = nullptr;
	size_t len = 0;

	if (!_dupenv_s(&pbuf, &len, "appdata") && pbuf && strnlen_s(pbuf, MAX_PATH))
	{
		std::string settings_path;
		settings_path.append(pbuf);
		settings_path.append(V("\\Hpp Hack CS16\\"));
		CreateDirectory(settings_path.c_str(), 0);
		free(pbuf);
		return settings_path;
	}

	Utils::TraceLog(V("> %s: failed to get settings path.\n"), V(__FUNCTION__));
	return std::string();
}

HWND GetWindowHandle()
{
	HWND data;

	EnumWindows([](HWND hWnd, LPARAM lParam)
	{
		HWND& data = *(HWND*)lParam;

		DWORD dwProcessId;
		GetWindowThreadProcessId(hWnd, &dwProcessId);

		if (GetCurrentProcessId() != dwProcessId || !(!GetWindow(hWnd, GW_OWNER) && IsWindowVisible(hWnd)))
			return TRUE;

		data = hWnd;
		return FALSE;

	}, (LPARAM)&data);

	if (!data || data == INVALID_HANDLE_VALUE)
		Utils::TraceLog(V("> %s: window handle not found.\n"), V(__FUNCTION__));

	return data;
}

CGlobals::CGlobals()
{
	m_dbInjectedTime = static_cast<double>(clock());

	m_sSettingsPath = GetSettingsPath();

	if (!m_sSettingsPath.empty())
	{
		m_sDebugFile = m_sSettingsPath + V("debug.log");
		m_sSystemDisk = m_sSettingsPath.substr(0, m_sSettingsPath.find(':'));
		m_dwProcessId = GetCurrentProcessId();
		m_hWnd = GetWindowHandle();

		DeleteFile(m_sDebugFile.c_str());
	}
}