#include "main.h"
#include "blackbone/BlackBone/Config.h"
#include "blackbone/BlackBone/Process/Process.h"
#include "blackbone/BlackBone/PE/PEImage.h"
#include "blackbone/BlackBone/Misc/Utils.h"
#include "blackbone/BlackBone/Patterns/PatternSearch.h"

Globals g;
std::vector<std::string> g_BuildsList;

enum InjectionStatesFlags
{
	Injection_None = 1 << 1,
	Injection_Download = 1 << 2,
	Injection_LookingProcess = 1 << 3,
	Injection_Injected = 1 << 4
};

typedef int InjectionState;
InjectionState g_IntectingState = Injection_None;

void DrawPoint(float angle, float radius, float x, float y, float size, const ImColor& color)
{
	ImVec2 position = ImVec2();

	position.x = x + ((x - radius) - x) * cos(angle) - ((y - radius) - y) * sin(angle);
	position.y = y + ((y - radius) - y) * cos(angle) + ((x - radius) - x) * sin(angle);

	ImGui::GetCurrentWindow()->DrawList->AddCircleFilled(position, size, color);
}

void DrawLoading(float x, float y, float alpha)
{
	static DWORD dwOneTick = 0;
	const DWORD dwCurrentTime = GetTickCount();

	if (!dwOneTick)
		dwOneTick = dwCurrentTime;

	const float flBeginAngle = D3DX_PI / 4.f;
	const float flTimeInterval = 100.f;
	const float flPointDistance = 0.78f;

	float flAngle = flBeginAngle + (dwCurrentTime - dwOneTick) * (D3DX_PI * 0.1f / flTimeInterval);

	for (int i_point = 0; i_point < 8; ++i_point)
		DrawPoint(flAngle + (flPointDistance * i_point), 10, x, y, 3, ImVec4(0.35f, 0.35f, 0.35f, alpha));
}

void SaveToClipboard(const char* data) noexcept
{
	if (OpenClipboard(g_hWnd))
	{
		EmptyClipboard();

		HGLOBAL hgBuff = GlobalAlloc(GMEM_DDESHARE, strlen(data) + 1);

		if (hgBuff)
		{
			char* cBuff = static_cast<char*>(GlobalLock(hgBuff));

			if (cBuff)
			{
				strcpy(cBuff, data);
				GlobalUnlock(hgBuff);
				SetClipboardData(CF_TEXT, hgBuff);
			}
		}

		CloseClipboard();
	}
}

std::string GetSettingsPath()
{
	std::string sSettingsPath = {};

	sSettingsPath.append(getenv("appdata"));
	sSettingsPath.append("\\" + g_BuildInfo[g.m_iCurrentBuild].m_sConfigFolder + "\\");

	CreateDirectoryA(sSettingsPath.c_str(), nullptr);

	return sSettingsPath;
}

std::string timestampToString(const time_t rawtime)
{
	struct tm * dt;
	char cBuffer[30] = { 0 };
	dt = localtime(&rawtime);
	strftime(cBuffer, sizeof(cBuffer), "%d.%m.%Y", dt);
	return std::string(cBuffer);
}

DWORD FindProcessId(const std::string& sProcessName) noexcept
{
	PROCESSENTRY32 processInfo;
	processInfo.dwSize = sizeof(processInfo);

	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (hSnapshot == INVALID_HANDLE_VALUE)
		return 0;

	Process32First(hSnapshot, &processInfo);

	if (!sProcessName.compare(processInfo.szExeFile))
	{
		CloseHandle(hSnapshot);
		return processInfo.th32ProcessID;
	}

	while (Process32Next(hSnapshot, &processInfo))
	{
		if (!sProcessName.compare(processInfo.szExeFile))
		{
			CloseHandle(hSnapshot);
			return processInfo.th32ProcessID;
		}
	}

	CloseHandle(hSnapshot);
	return 0;
}

#if 0
DWORD DownloadLatestUpdateThread(LPVOID)
{
	g.m_sResponse.clear();

	const std::string sUrl = "/hpp/download.php?id=" + g_BuildInfo[g.m_iSavedCurrentBuild].m_sDownloadID;// ? " + g_BuildInfo[g.m_iSavedCurrentBuild].m_sUrl + "&hwid = " + g_LicenseInfo.m_sHardwareID

	g.m_sResponse = Security::GetUrlData(sUrl);

	g.m_dwCurrentTime = GetTickCount();

	g_IntectingState = Injection_LookingProcess;
	return 0;
}
#endif

DWORD InjectToProcessThread(LPVOID)
{
	//static const char uszPatch[] = { 0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37,0x37 };
	//static blackbone::PatternSearch ps{ "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90" };

	blackbone::Process process;
	process.Attach(g.m_dwProcessID);

#if 0
	if (g.m_sResponse.size())
	{
		Crypto::base64_decode(g.m_sResponse);

		blackbone::Process process;
		process.Attach(g.m_dwProcessID);

		/*if (g_BuildInfo[g.m_iSavedCurrentBuild].m_sGameName == "Counter-Strike 1.6")
		{
			auto blacbone_module = process.modules().GetModule(L"hw.dll");

			if (!blacbone_module)
				blacbone_module = process.modules().GetModule(L"engine.dll");

			if (blacbone_module)
			{
				std::vector<blackbone::ptr_t> results;

				auto &memory = process.memory();
				ps.SearchRemote(process, blacbone_module->baseAddress, blacbone_module->size, results);

				if (!results.empty())
				{
					const auto address = results.front();

					DWORD dwOldProtect = 0;

					if (NT_SUCCESS(memory.Protect(address, sizeof(uszPatch), PAGE_READWRITE, &dwOldProtect)))
					{
						memory.Write(results.front(), "\x37\x37\x37\x37\x37\x37\x37\x37\x37\x37");
						memory.Protect(address, sizeof(uszPatch), dwOldProtect);

						auto image = process.mmap().MapImage(g.m_sResponse.size(), (void*)(g.m_sResponse.c_str()), 
							false, blackbone::CreateLdrRef | blackbone::RebaseProcess | blackbone::NoDelayLoad | blackbone::NoTLS);
					}
				}
			}
		}
		else {*/
			auto image = process.mmap().MapImage(g.m_sResponse.size(), (void*)(g.m_sResponse.c_str()),
				false, blackbone::CreateLdrRef | blackbone::RebaseProcess | blackbone::NoDelayLoad | blackbone::NoTLS);
		//}
	}

	g.m_sResponse.clear();
	g.m_dwCurrentTime = 0;
	g.m_dwProcessID = 0;
	g.m_iSavedCurrentBuild = 0;
	g.m_bIsDownloadThreadStart = false;
	g.m_bIsInjectingThreadStart = false;
#endif

	g_IntectingState = Injection_None;
	
	return 0;
}

void StartPoint()
{
	if (g_LicenseInfo.m_bIsUpdated)
	{
		g_LicenseInfo.m_bIsUserValid ? g_pWindow->SwapWindow("General") : g_pWindow->SwapWindow("HardwareID");
	}
	else
	{
		const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

		if (pFont == nullptr)
			return;

		ImVec2 vecTextSize = ImVec2();

		DrawLoading(WinWidth * 0.5f, WinHeight * 0.5f, 1);
		ImGui::Spacing(45);

		vecTextSize = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, g_LicenseInfo.m_sMainText.c_str());

		ImGui::SameLine((WinWidth - vecTextSize.x) * 0.5f);
		ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1.00f), g_LicenseInfo.m_sMainText.c_str());
		
		/*if (g_LicenseInfo.m_bIsAviableUpdate)
		{
			ImGui::Spacing(2);

			static float progress = 0.f;
			progress = static_cast<float>(g.m_dwDownloadedBytes) / 3072000.f;

			ImGui::SameLine((WinWidth - 200) * 0.5f);
			ImGui::PushItemWidth(200);
			ImGui::ProgressBar(progress, ImVec2(0.f, 0.f));
			ImGui::PopItemWidth();
		}*/
	}
}

void HardwareID()
{
	static DWORD dwCopiedTime = 0;

	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	ImVec2 vecTextSize = ImVec2();
	const float fItemWidth = 350.f;
	const ImVec2 vecButtonSize = ImVec2(120, 16);

	ImGui::Spacing(12);
	ImGui::SameLine((WinWidth - 86) * 0.5f);
	ImGui::Image(g_pD3DTexture_Logotype, ImVec2(86, 109), ImVec2(), ImVec2(1, 1), ImVec4(0.31f, 0.31f, 0.31f, 1.00f));
	ImGui::Spacing(12);

	const char* szText = "Your Hardware Identifier:";
	vecTextSize = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, szText);

	ImGui::SameLine((WinWidth - vecTextSize.x) * 0.5f);
	ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1.f), szText);
	ImGui::Spacing(2);
	ImGui::SameLine((WinWidth - fItemWidth) * 0.5f);
	ImGui::PushItemWidth(fItemWidth);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.31f, 0.31f, 0.31f, 1.f));
	ImGui::InputText("##HardwareID", (char*)g_LicenseInfo.m_sHWID.c_str(), g_LicenseInfo.m_sHWID.size() + 1, ImGuiInputTextFlags_ReadOnly | ImGuiInputTextFlags_AutoSelectAll);
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	ImGui::Spacing(2);
	ImGui::SameLine((WinWidth - vecButtonSize.x) * 0.5f);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.f, 1.f, 1.f, 1.f));

	if (ImGui::Button("Copy to Clipboard", vecButtonSize))
	{
		SaveToClipboard(g_LicenseInfo.m_sHWID.c_str());
		dwCopiedTime = GetTickCount();
	}

	ImGui::PopStyleColor();
	ImGui::Spacing(2);

	if (GetTickCount() < dwCopiedTime + 2250)
	{
		szText = "Hardware ID is copied";
		vecTextSize = pFont->CalcTextSizeA(pFont->FontSize, FLT_MAX, 0, szText);

		ImGui::SameLine((WinWidth - vecTextSize.x) * 0.5f);

		float flAlpha = 1.f;

		if (GetTickCount() > dwCopiedTime + 2000)
		{
			flAlpha = fInterp(static_cast<float>(dwCopiedTime + 2000), static_cast<float>(GetTickCount()), 
				static_cast<float>(dwCopiedTime + 2250), 1.f, 0.f);
		}

		ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, flAlpha), szText);
	}

	if (g_LicenseInfo.m_bIsUpdated && g_LicenseInfo.m_bIsUserValid)
	{
		g_pWindow->SwapWindow("StartPoint");

		//g_LicenseInfo.m_bIsUpdated = false;
	}
}

void General()
{
	const ImFont* pFont = ImGui::GetIO().Fonts->Fonts[0];

	if (pFont == nullptr)
		return;

	const ImVec2 vecButtonSize = ImVec2(120, 16);

	ImGui::Spacing(3);
	ImGui::SameLine(40);
	ImGui::BeginGroup();
	ImGui::Image(g_pD3DTexture_Logotype, ImVec2(72, 90), ImVec2(0, 0), ImVec2(1, 1), ImVec4(0.31f, 0.31f, 0.31f, 1.f));
	ImGui::Spacing(2);
	ImGui::SameLine(-25);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

	if (ImGui::Button("Copy Hardware ID", vecButtonSize))
		SaveToClipboard(g_LicenseInfo.m_sHWID.c_str());

	ImGui::Spacing();
	ImGui::SameLine(-25);

	if (ImGui::Button("Open config folder", vecButtonSize))
		ShellExecuteA(nullptr, "open", GetSettingsPath().c_str(), nullptr, nullptr, SW_RESTORE);

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::SameLine(0, 15);
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.f, 0.f, 0.f, 0.08f));
	ImGui::VerticalSeparator();
	ImGui::PopStyleColor();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::PushItemWidth(250);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.31f, 0.31f, 0.31f, 1.f));
	ImGui::ListBoxArray("##BuildsList", &g.m_iCurrentBuild, g_BuildsList, 9);
	ImGui::PopStyleColor();
	ImGui::PopItemWidth();
	ImGui::EndGroup();
	ImGui::SameLine();
	ImGui::BeginGroup();
	ImGui::PushFont(Lucida11px);
	ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1.f), "Information");
	ImGui::PopFont();
	ImGui::Spacing(2);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.31f, 0.31f, 0.31f, 1.f));
	if (!g_BuildInfo.empty())
	{
		ImGui::Text("Game: %s", g_BuildInfo[g.m_iCurrentBuild].m_sGameName.c_str());
		ImGui::Text("Version: %s", g_BuildInfo[g.m_iCurrentBuild].m_sVersion.c_str());
	}
	ImGui::Text("Status:");
	ImGui::SameLine();
	ImGui::TextColored(ImVec4(0.31f, 0.6f, 0.31f, 1.f), "Undetectable");

	ImGui::Spacing();

	/*const std::string sInfo = g_BuildInfo[g.m_iCurrentBuild].m_sInformation;
	
	for (auto j = 1; j < SHRT_MAX; j++)
	{
		std::string sTmp = FindArgument(sInfo, j, "^");

		if (sTmp.length() < 2)
			break;

		ImGui::Text(sTmp.c_str());
	}*/

	ImGui::PopStyleColor();
	ImGui::EndGroup();
	ImGui::Spacing(3);
	ImGui::SameLine(15);
	ImGui::BeginGroup();
	ImGui::PushStyleColor(ImGuiCol_Separator, ImVec4(0.f, 0.f, 0.f, 0.08f));
	ImGui::Separator(WinWidth - 15);
	ImGui::PopStyleColor();
	ImGui::Spacing(3);
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.31f, 0.31f, 0.31f, 1.f));
	ImGui::PushFont(Lucida11px);
	ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, 1.f), "Instruction");
	ImGui::PopFont();
	ImGui::Spacing(2);
	ImGui::Text("1. Click the button \"Download & Inject\".");
	ImGui::Text("2. Wait until the current version of build is downloaded.");
	ImGui::Text("3. Start the game.");
	ImGui::Text("4. After the injection you will see a menu of cheat.");
	ImGui::Spacing(9);

	if (g_IntectingState & Injection_None)
	{
		ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1, 1, 1, 1));

		if (ImGui::Button("Download & Inject", vecButtonSize))
			g_IntectingState = Injection_Download;

		ImGui::PopStyleColor();

		g.m_iSavedCurrentBuild = g.m_iCurrentBuild;
	}
	else
	{
		static DWORD dwFoundProcessDelay = 0;

		if (!g.m_dwCurrentTime)
			g.m_dwCurrentTime = GetTickCount();

		const float flFadeTime = 500.f;
		const float flAlpha = fInterp(static_cast<float>(g.m_dwCurrentTime), static_cast<float>(GetTickCount()), 
			static_cast<float>(g.m_dwCurrentTime + flFadeTime), 0.f, 1.f);

		DrawLoading(30, WinHeight - 30, flAlpha);

		ImGui::SameLine(50);

		if (g_IntectingState & Injection_Download)
		{
#if 0
			if (!g.m_bIsDownloadThreadStart)
			{
				CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)DownloadLatestUpdateThread, nullptr, 0, nullptr);
				g.m_bIsDownloadThreadStart = true;
			}

			const double dbSize = g.m_dwDownloadedBytes * 0.0000005;

			ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, flAlpha), "Downloading latest update... (%.1f MB)", dbSize);
#endif
		}
		else if (g_IntectingState & Injection_LookingProcess)
		{
			if (g.m_sResponse.size())
			{
				ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, flAlpha), "Waiting for game launch...");

				if (g.m_dwProcessID)
				{
					g.m_dwCurrentTime = GetTickCount();
					g_IntectingState = Injection_Injected;
				}
				else
				{
					g.m_dwProcessID = FindProcessId(g_BuildInfo[g.m_iSavedCurrentBuild].m_sProcessName);

					if (!g.m_dwProcessID && g_BuildInfo[g.m_iSavedCurrentBuild].m_sGameName == "Counter-Strike 1.6")
						g.m_dwProcessID = FindProcessId("cs.exe");

					dwFoundProcessDelay = GetTickCount();
				}
			}
#if 0
			else
			{
				ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, flAlpha), "Error. Retrying to download...");

				if (g.m_dwCurrentTime + 2500 < GetTickCount())
					g_IntectingState = Injection_Download;

				g.m_sResponse.clear();
			}
#endif

			g.m_bIsDownloadThreadStart = false;
		}
		else if (g_IntectingState & Injection_Injected)
		{
			if (dwFoundProcessDelay + 4900 <= GetTickCount())
			{
				if (!g.m_bIsInjectingThreadStart)
				{
					CreateThread(nullptr, 0, (LPTHREAD_START_ROUTINE)InjectToProcessThread, nullptr, 0, nullptr);
					g.m_bIsInjectingThreadStart = true;
				}
			}

			ImGui::TextColored(ImVec4(0.25f, 0.25f, 0.25f, flAlpha), "Injecting...");
		}
	}

	ImGui::PopStyleColor();
	ImGui::EndGroup();

	ImGui::SameLine(WinWidth - 105);
	ImGui::BeginGroup();
	ImGui::Spacing(32);

	ImGui::PushStyleColor(ImGuiCol_Button, ImVec4());
	ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.00f, 0.25f, 1.00f, 0.25f));
	ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4(0.00f, 0.25f, 1.00f, 0.25f));
	ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.00f, 0.25f, 1.00f, 1.00f));
	ImGui::PushStyleColor(ImGuiCol_Border, ImVec4());
	ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(8, 1));

	ImGui::SameLine(47);

	if (ImGui::ButtonLink("hpp.ovh"))
		ShellExecuteA(nullptr, "open", "https://hpp.ovh", nullptr, nullptr, SW_SHOW);

	if (ImGui::ButtonLink("vk.com/hpp_hack"))
		ShellExecuteA(nullptr, "open", "http://vk.com/hpp_hack", nullptr, nullptr, SW_SHOW);

	ImGui::PopStyleColor(5);
	ImGui::PopStyleVar();
	ImGui::EndGroup();
}