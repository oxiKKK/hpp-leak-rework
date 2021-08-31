#include "main.h"

bool HideThread(HANDLE hThread)
{
	HMODULE NtDll = GetModuleHandle("ntdll.dll");

	if (!NtDll)
		return false;

	pNtSetInformationThread NtSIT = (pNtSetInformationThread)GetProcAddress(NtDll, "NtSetInformationThread");

	if (!NtSIT)
		return false;

	NTSTATUS Status = hThread ? NtSIT(hThread, 0x11, 0, 0) : NtSIT(GetCurrentThread(), 0x11, 0, 0);

	return Status == 0;
}

__forceinline std::string FindArgument(std::string sInput, int iNumber, std::string sIterator = ";")
{
	std::string sTmp = sInput;
	size_t iCount = 0;

	for (auto i = 0; i < iNumber; i++)
	{
		if (iCount && sTmp.length() - iCount > 0)
			sTmp = sInput.substr(iCount, sTmp.length() - iCount);

		auto pos = sTmp.find(sIterator);

		if (pos == std::string::npos)
			return {};

		sTmp = sTmp.substr(0, pos);

		iCount += sTmp.length() + 1;
	}

	return sTmp;
}

__forceinline std::string XorEncryptDecrypt(const std::string sInput)
{
	static const char szKey[] = { 'T','Z','G','G','Y','Y','g','S','1','K','t','k','E','z','F','x','d','u','v','q','g','r','2','y','z','J','w','Q','A','Z','W','B' };

	std::string sOutput = sInput;

	for (auto i = 0; i < sInput.size(); i++)
		sOutput[i] = sInput[i] ^ szKey[i % (sizeof(szKey) / sizeof(char))];

	return sOutput;
}

__forceinline std::string HexToString(const std::string& sInput)
{
	static const char* const szLut = "0123456789ABCDEF";
	auto len = sInput.length();

	if (len & 1)
		return sInput;

	std::string sOutPut;
	
	sOutPut.reserve(len / 2);

	for (size_t i = 0; i < len; i += 2)
	{
		auto a = sInput[i];
		auto p = std::lower_bound(szLut, szLut + 16, a);

		if (*p != a) 
			return sInput;

		auto b = sInput[i + 1];
		auto q = std::lower_bound(szLut, szLut + 16, b);

		if (*q != b) 
			return sInput;

		sOutPut.push_back(((p - szLut) << 4) | (q - szLut));
	}

	return sOutPut;
}

__forceinline std::string StringToHex(const std::string& sInput)
{
	static const char* const szLut = "0123456789ABCDEF";
	auto len = sInput.length();

	std::string sOutput;

	sOutput.reserve(2 * len);

	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = sInput[i];
		sOutput.push_back(szLut[c >> 4]);
		sOutput.push_back(szLut[c & 15]);
	}

	return sOutput;
}

void LicenseCheck()
{
#ifdef LICENSE
	VM_START;
	SYSTEMTIME SystemTime;
	GetSystemTime(&SystemTime);

	STR_ENCRYPT_START;
	if (SystemTime.wYear != 2018 || SystemTime.wMonth > 10 || SystemTime.wMonth < 9)
	{
		g_Offsets.Error("Incorrect system time.");
		return;
	}

	HRESULT hResult = CoInitializeEx(0, COINIT_MULTITHREADED);

	if (FAILED(hResult))
		return;

	IWbemLocator *pLoc = NULL;

	hResult = CoCreateInstance(CLSID_WbemLocator, 0, CLSCTX_INPROC_SERVER, IID_IWbemLocator, (LPVOID *)&pLoc);

	if (FAILED(hResult))
		return;

	IWbemServices *pSvc = NULL;

	hResult = pLoc->ConnectServer(bstr_t("ROOT\\CIMV2"), NULL, NULL, 0, NULL, 0, 0, &pSvc);

	if (FAILED(hResult))
		return;

	hResult = CoSetProxyBlanket(pSvc, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	if (FAILED(hResult))
		return;

	IEnumWbemClassObject* pEnumerator = NULL;
	hResult = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hResult))
		return;

	std::wstringstream wssHWID;
	wssHWID << time(NULL) << ';';
	wssHWID << g_Globals.m_dwProcessID << ';';

	IWbemClassObject *pclsObj = NULL;
	ULONG uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (0 == uReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pclsObj->Release();
	}

	pEnumerator = NULL;
	hResult = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hResult))
		return;

	pclsObj = NULL;
	uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (0 == uReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		hr = pclsObj->Get(L"SerialNumber", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Version", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pclsObj->Release();
	}

	pEnumerator = NULL;
	hResult = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hResult))
		return;

	pclsObj = NULL;
	uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (0 == uReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		hr = pclsObj->Get(L"InstallDate", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		hr = pclsObj->Get(L"Manufacturer", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pclsObj->Release();
	}

	pEnumerator = NULL;
	hResult = pSvc->ExecQuery(bstr_t("WQL"), bstr_t("SELECT * FROM Win32_ComputerSystemProduct"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, NULL, &pEnumerator);

	if (FAILED(hResult))
		return;

	pclsObj = NULL;
	uReturn = 0;

	while (pEnumerator)
	{
		HRESULT hr = pEnumerator->Next(WBEM_INFINITE, 1, &pclsObj, &uReturn);

		if (0 == uReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		hr = pclsObj->Get(L"UUID", 0, &vtProp, 0, 0);
		wssHWID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pclsObj->Release();
	}

	pSvc->Release();
	pLoc->Release();
	pEnumerator->Release();
	CoUninitialize();

	std::wstring wsWide(wssHWID.str());

	wssHWID.clear();

	std::string sHashedHWID(wsWide.begin(), wsWide.end());

	wsWide.clear();

	sHashedHWID = StringToHex(XorEncryptDecrypt(sHashedHWID));

	std::string sUrl = "/hpp/server.php?product=1&type=2&hwid=" + sHashedHWID;
	static std::string sActivation("activated;");

repeat_check_link:

	std::string sRespone = GetUrlData(sUrl);

	if (sRespone.find(sActivation) == std::string::npos)
	{
		g_pConsole->DPrintf("[hpp] Respone: %s\n", sRespone.c_str());
		
		static auto MaxAttempts = 3;

		if (MaxAttempts > 0)
		{
			MaxAttempts--;
			Sleep(3000);
			goto repeat_check_link;
		}
		g_Offsets.Error(sRespone.c_str());
		return;
	}

	STR_ENCRYPT_END;
	sRespone.erase(sRespone.begin(), sRespone.end() - sRespone.length() + sActivation.length());
	sRespone = XorEncryptDecrypt(HexToString(sRespone));

	std::string sTime = FindArgument(sRespone, 1);
	std::string sProcessID = FindArgument(sRespone, 2);

	auto DifferenceTimes = abs(time(NULL) - std::stoi(sTime));

	if (DifferenceTimes < 86400 && std::stoi(sProcessID) == g_Globals.m_dwProcessID)
	{
		g_Offsets.m_dwCloudOffsets[0] = std::stoi(FindArgument(sRespone, 3));
		g_Offsets.m_dwCloudOffsets[1] = std::stoi(FindArgument(sRespone, 4));
	}
	else
		return;

	sRespone.clear();

	static auto bNeedFindHardware = true;

	if (bNeedFindHardware)
		goto findhardware_link;

	return;
findhardware_link:
	bNeedFindHardware = false;
	while (!g_Offsets.FindHardware())
		Sleep(100);
	VM_END;
#endif
}

std::string GetUrlData(const std::string& sURL)
{
	//VM_START;
	std::string sRequestData = "";
	HINTERNET hIntSession, hHttpSession, hHttpRequest;
	STR_ENCRYPT_START;
	if ((hIntSession = InternetOpenA("Hpp Hack", INTERNET_OPEN_TYPE_DIRECT, NULL, NULL, 0)) == NULL)
	{
		InternetCloseHandle(hIntSession);
		return sRequestData;
	}

	if ((hHttpSession = InternetConnectA(hIntSession, "hpp.ovh", INTERNET_DEFAULT_HTTPS_PORT, 0, 0, INTERNET_SERVICE_HTTP, 0, NULL)) == NULL)
	{
		InternetCloseHandle(hHttpSession);
		return sRequestData;
	}

	if ((hHttpRequest = HttpOpenRequestA(hHttpSession, "GET", sURL.c_str(), "HTTP/1.1", 0, 0, INTERNET_FLAG_SECURE | INTERNET_FLAG_IGNORE_CERT_CN_INVALID, 0)) == NULL)
	{
		InternetCloseHandle(hHttpRequest);
		return sRequestData;
	}
	STR_ENCRYPT_END;
	if (HttpSendRequestA(hHttpRequest, 0, 0, 0, 0))
	{
		char cBuff[1024] = { 0 };
		DWORD dwRead = 0;

		while (InternetReadFile(hHttpRequest, cBuff, sizeof(cBuff) - 1, &dwRead) && dwRead)
			sRequestData.append(cBuff, dwRead);
	}

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);
	//VM_END;
	return sRequestData;
}

DWORD FindProcessId(const std::string& sProcessName)
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