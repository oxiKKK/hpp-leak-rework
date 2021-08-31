#include "main.h"

#if 0
static constexpr auto base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";

std::unique_ptr<WebSocket> websocket;

bool Socket::Create()
{
	WSADATA wsaData;

	const auto rc = WSAStartup(MAKEWORD(2, 2), &wsaData);

	if (rc)
	{
		Security::Error(V("%s: WSAStartup error %i.\n"), V(__FUNCTION__), rc);
		return false;
	}

	websocket = std::unique_ptr<WebSocket>(WebSocket::from_url(V("ws://sock.hpp.ovh:7777/loader")));

	if (!websocket.get())
	{
		websocket = std::unique_ptr<WebSocket>(WebSocket::from_url(V("ws://sock2.hpp.ovh:7777/loader")));

		if (!websocket.get())
		{
			Security::Error(V("%s: failed connect to server.\n"), V(__FUNCTION__));
			return false;
		}
	}

	Security::Error(V("> Socket: created.\n"));

	return true;
}

void HandleMessage(const std::string& message)
{
	Security::Error(message.c_str());

	if (!message.compare(V("terminate")))
	{
		TerminateProcess(GetCurrentProcess(), 0);
	}
	else if (message.find(V("sv:[gamelist]")) != std::string::npos)
	{
		auto response = message.substr(13, message.length());

		g_BuildInfo.clear();
		g_BuildsList.clear();

		if (response.find(V("cs16")) != std::string::npos)
		{
			g_Build.Add("hpp_cs16", "V6 [beta]", "Counter-Strike 1.6", "cs16", "Hpp Hack CS16", "hl.exe", "Support:^- Valve Anti-Cheat^- Server Side Anti-Cheat^");

			for (size_t i = 0; i < g_BuildInfo.size(); ++i)
				g_BuildsList.push_back(g_BuildInfo[i].m_sBuildName);
		}
	}
	else if (message.find(V("sv:[loader]")) != std::string::npos)
	{
		auto response = message.substr(11, message.length());

		int nServerVersion = std::stoi(response);

		if (nServerVersion > VERSION)
		{
			g_LicenseInfo.m_bIsUserValid = false;
			g_LicenseInfo.m_bIsUpdated = false;
			g_LicenseInfo.m_sMainText = "Updating";

			char cBuffer[MAX_PATH] = { 0 };

			std::string sResponse = {};
			std::string sFileName = {};

			RtlSecureZeroMemory(cBuffer, sizeof(cBuffer));
			GetModuleFileNameA(g_hInstance, cBuffer, sizeof(cBuffer));

			sFileName = cBuffer;

			if (rename(sFileName.c_str(), std::string(sFileName + ".tmp").c_str()))
			{
				MessageBoxA(g_hWnd, "Can not update launcher #1", "Error", MB_OK);
				Shutdown();
			}

			FILE* pFile = fopen(sFileName.c_str(), "wb+");

			if (pFile == nullptr)
			{
				MessageBoxA(g_hWnd, "Can not update launcher #2", "Error", MB_OK);
				Shutdown();
			}

			sResponse.append(Security::GetUrlData("/hpp/download.php?id=loader"));

			if (sResponse.size())
			{
				Crypto::base64_decode(sResponse);
				fwrite(sResponse.c_str(), sizeof(char), sResponse.size(), pFile);
			}

			fclose(pFile);

			sResponse.clear();

			ShellExecuteA(nullptr, nullptr, sFileName.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
			Shutdown();
		}
	}
	else if (message.find(V("sv:[hwid]")) != std::string::npos)
	{
		auto response = message.substr(9, message.length());

		for (size_t i = 0; i < 3; i++)
		{
			Crypto::base64_decode(response);
			Crypto::xor_crypt(response);
		}

		const auto sv_time = response.substr(0, response.find(';'));
		const auto sv_processid = response.substr(sv_time.length() + 1, response.find(';', sv_time.length() + 1) - sv_time.length() - 1);

		const auto sv_time_converted = std::stoi(sv_time);
		const auto sv_processid_converted = std::stoi(sv_processid);

#if !defined(LICENSING) || defined(DEBUG)
		/*g_pConsole->DPrintf("> %s: sv_time: %s %i\n", __FUNCTION__, sv_time.c_str(), sv_time_converted);
		g_pConsole->DPrintf("> %s: sv_processid: %s %i\n", __FUNCTION__, sv_processid.c_str(), sv_processid_converted);*/
#endif
		const auto client_time = static_cast<int>(std::time(0));
		const auto time_diff = abs(client_time - sv_time_converted);

		if (time_diff < (24 * 60 * 60) && sv_processid_converted == g_LicenseInfo.m_dwProcessId)
		{
			const auto forum_user_hwid = response.substr(sv_time.length() + 1 + sv_processid.length() + 1, INT_MAX);
			auto cl_hwid = g_LicenseInfo.m_sHWID;

			for (size_t i = 0; i < 3; i++)
			{
				Crypto::base64_decode(cl_hwid);
				Crypto::xor_crypt(cl_hwid);
			}

			cl_hwid = cl_hwid.substr(3 + sv_time.length() + 1 + sv_processid.length() + 1, INT_MAX); // 3 = 'CL:'

			//Security::Error("forum_user_hwid: %s\n\ncl_hwid: %s", forum_user_hwid.c_str(), cl_hwid.c_str());

			if (forum_user_hwid == cl_hwid)
			{
#if !defined(LICENSING) || defined(DEBUG)
				/*g_pConsole->DPrintf("> %s: forum_user_hwid: %s.\n", __FUNCTION__, forum_user_hwid.c_str());
				g_pConsole->DPrintf("> %s: cl_hwid: %s.\n", __FUNCTION__, cl_hwid.c_str());*/
#endif
				g_LicenseInfo.m_bIsUserValid = true;
				return;
			}
		}
	}
}

void Security::Error(const char* fmt, ...)
{
	/*char buf[4096] = { 0 };

	va_list args;
	va_start(args, fmt);
	vsprintf_s(buf, fmt, args);
	va_end(args);

	MessageBoxA(g_hWnd, buf, V("Error"), MB_OK);*/
}

bool Security::SendHardwareID()
{
	/*if (!websocket.get())
		return false;

	HRESULT hResult = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hResult))
		return false;

	IWbemLocator* pLoc;
	hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID*)& pLoc);

	if (FAILED(hResult))
	{
		CoUninitialize();
		return false;
	}

	IWbemServices* pSvc;
	hResult = pLoc->ConnectServer(bstr_t(V("ROOT\\CIMV2")), NULL, NULL, 0, NULL, 0, 0, &pSvc);

	if (FAILED(hResult))
	{
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	hResult = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	if (FAILED(hResult))
	{
		pSvc->Release();
		pLoc->Release();
		CoUninitialize();
		return false;
	}

	std::stringstream ssHWID;
	ssHWID << V("CL:") << std::time(0) << ';' << g_LicenseInfo.m_dwProcessId << ';';

	IEnumWbemClassObject* pEnumerator;
	IWbemClassObject* pClassObj;
	ULONG ulReturn = 0;
	VARIANT vtProp;

	hResult = pSvc->ExecQuery(bstr_t(V("WQL")), bstr_t(V("SELECT * FROM Win32_BaseBoard")), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hResult))
	{
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

			if (0 == ulReturn)
				break;

			hr = pClassObj->Get(W_V(L"SerialNumber"), 0, &vtProp, 0, 0);
			ssHWID << bstr_t(vtProp.bstrVal) << ';';
			VariantClear(&vtProp);
			pClassObj->Release();
		}
	}

	hResult = pSvc->ExecQuery(bstr_t(V("WQL")), bstr_t(V("SELECT * FROM Win32_BIOS")), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hResult))
	{
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

			if (0 == ulReturn)
				break;

			hr = pClassObj->Get(W_V(L"SerialNumber"), 0, &vtProp, 0, 0);
			ssHWID << bstr_t(vtProp.bstrVal) << ';';
			VariantClear(&vtProp);
			pClassObj->Release();
		}
	}

	hResult = pSvc->ExecQuery(bstr_t(V("WQL")), bstr_t(V("SELECT * FROM Win32_ComputerSystemProduct")), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hResult))
	{
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

			if (0 == ulReturn)
				break;

			hr = pClassObj->Get(W_V(L"UUID"), 0, &vtProp, 0, 0);
			ssHWID << bstr_t(vtProp.bstrVal) << ';';
			VariantClear(&vtProp);
			pClassObj->Release();
		}
	}

	hResult = pSvc->ExecQuery(bstr_t(V("WQL")), bstr_t(V("SELECT * FROM Win32_DesktopMonitor")), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hResult))
	{
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

			if (0 == ulReturn)
				break;

			hr = pClassObj->Get(W_V(L"PNPDeviceID"), 0, &vtProp, 0, 0);

			if (vtProp.bstrVal)
			{
				std::string PNPDeviceID = (const char*)bstr_t(vtProp.bstrVal);
				PNPDeviceID = std::regex_replace(PNPDeviceID, std::regex(V("&\\d+&")), V("&dynamic&"));
				ssHWID << PNPDeviceID << ';';
			}

			VariantClear(&vtProp);
			pClassObj->Release();
		}
	}

	hResult = pSvc->ExecQuery(bstr_t(V("WQL")), bstr_t(V("SELECT * FROM Win32_DiskDrive")), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (SUCCEEDED(hResult))
	{
		while (pEnumerator)
		{
			HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

			if (0 == ulReturn)
				break;

			pClassObj->Get(W_V(L"MediaType"), 0, &vtProp, 0, 0);

			if (vtProp.bstrVal)
			{
				if (wcsstr(vtProp.bstrVal, W_V(L"Fixed")))
				{
					hr = pClassObj->Get(W_V(L"PNPDeviceID"), 0, &vtProp, 0, 0);

					if (vtProp.bstrVal)
					{
						std::string PNPDeviceID = (const char*)bstr_t(vtProp.bstrVal);
						PNPDeviceID = std::regex_replace(PNPDeviceID, std::regex(V("&\\d+&")), V("&dynamic&"));
						ssHWID << PNPDeviceID << '&';
					}

					hr = pClassObj->Get(W_V(L"SerialNumber"), 0, &vtProp, 0, 0);

					ssHWID << bstr_t(vtProp.bstrVal) << ';';
				}
			}

			VariantClear(&vtProp);
			pClassObj->Release();
		}
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

#if !defined(LICENSING) || defined(DEBUG)
	//g_pConsole->DPrintf("> %s: ssHWID: %s\n", __FUNCTION__, ssHWID.str());
#endif
	std::string sHWID(ssHWID.str());
	ssHWID.clear();
	sHWID = std::regex_replace(sHWID, std::regex(V("\\s+")), "");

#if !defined(LICENSING) || defined(DEBUG)
	//g_pConsole->DPrintf("> %s: sHWID: %s\n", __FUNCTION__, sHWID.c_str());
#endif
	for (unsigned i = 0; i < 3; i++)
	{
		Crypto::xor_crypt(sHWID);
		Crypto::base64_encode(sHWID);
	}

	websocket->send(V("cl:[hwid]") + sHWID);
	g_LicenseInfo.m_sHWID = sHWID;*/
	return true;
}

bool Socket::Update()
{
	if (websocket && websocket->getReadyState() != WebSocket::CLOSED)
	{
		websocket->poll();
		websocket->dispatch(HandleMessage);
		return true;
	}

	return false;
}

void Socket::Close()
{
	if (websocket.get())
	{
		websocket->close();
		WSACleanup();

		Security::Error(V("> Socket: closed.\n"));
	}
}

bool Socket::Reconnect()
{
	Security::Error(V("> Socket: reconnecting...\n"));

	Close();

	return Create();
}

bool Security::HideThread(HANDLE hThread)
{
	HMODULE NtDll = GetModuleHandle("ntdll.dll");

	if (!NtDll || NtDll == INVALID_HANDLE_VALUE)
		return false;

	using pNtSetInformationThread = NTSTATUS(NTAPI*)(HANDLE, UINT, PVOID, ULONG);
	const auto NtSIT = (pNtSetInformationThread)GetProcAddress(NtDll, "NtSetInformationThread");

	if (!NtSIT)
		return false;

	return NTSTATUS(hThread ? NtSIT(hThread, 0x11, 0, 0) : NtSIT(GetCurrentThread(), 0x11, 0, 0));
}

std::string Security::GetUrlData(const std::string& sURL)
{
	g.m_dwDownloadedBytes = 0;
	std::string sRequestData = "";
	HINTERNET hIntSession, hHttpSession, hHttpRequest;

	if ((hIntSession = InternetOpenA(NULL, INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		InternetCloseHandle(hIntSession);
		return sRequestData;
	}

	if ((hHttpSession = InternetConnectA(hIntSession, V("hpp.ovh"), INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL)) == NULL)
	{
		InternetCloseHandle(hHttpSession);
		return sRequestData;
	}

	if ((hHttpRequest = HttpOpenRequestA(hHttpSession, "GET", sURL.c_str(), "HTTP/1.1", 0, 0, INTERNET_FLAG_SECURE | INTERNET_FLAG_NO_CACHE_WRITE | INTERNET_FLAG_PRAGMA_NOCACHE, 0)) == NULL)
	{
		InternetCloseHandle(hHttpRequest);
		return sRequestData;
	}

	if (HttpSendRequestA(hHttpRequest, 0, 0, 0, 0))
	{
		char cBuff[1024] = { 0 };
		DWORD dwRead = 0;

		while (InternetReadFile(hHttpRequest, cBuff, sizeof(cBuff) - 1, &dwRead) && dwRead)
		{
			sRequestData.append(cBuff, dwRead);
			g.m_dwDownloadedBytes = sRequestData.size();
		}
	}

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);
	return sRequestData;
}

void Crypto::xor_crypt(std::string& data)
{
	const char key[] = { 'i','j','|','}','?','b','O','Q','$','Y','a','e','0','r','R','u','V','c','5','L','2','%','y','S','m','w','E','U','@','o','t','Z','A','6','G','q','k','p','8','3','v','i','g','v','Q','Z','|','g','@','5','L','u','x','5','~','K','o','v','@','f','d','K','A','k' };
	const auto key_len = sizeof(key);

	for (size_t i = 0; i < data.size(); i++)
		data[i] ^= key[i % key_len];
}

void Crypto::base64_encode(std::string& data)
{
	std::string out;

	int val = 0, valb = -6;

	for (unsigned char c : data)
	{
		val = (val << 8) + c;
		valb += 8;

		while (valb >= 0)
		{
			out.push_back(base64_chars[(val >> valb) & 0x3F]);
			valb -= 6;
		}
	}

	if (valb > -6)
		out.push_back(base64_chars[((val << 8) >> (valb + 8)) & 0x3F]);

	while (out.size() % 4)
		out.push_back('=');

	data = out;
}

void Crypto::base64_decode(std::string& data)
{
	std::string out;

	std::vector<int> T(256, -1);

	for (int i = 0; i < 64; i++)
		T[base64_chars[i]] = i;

	int val = 0, valb = -8;

	for (unsigned char c : data)
	{
		if (T[c] == -1)
			break;

		val = (val << 6) + T[c];
		valb += 6;

		if (valb >= 0)
		{
			out.push_back(char((val >> valb) & 0xFF));
			valb -= 8;
		}
	}

	data = out;
}

#endif