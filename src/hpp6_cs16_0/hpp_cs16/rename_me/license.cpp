#include "main.h"

std::string FindArgument(std::string input, int num, std::string iterator = ";")
{
	std::string sTmp = input;
	auto iCount = 0;

	for (auto i = 0; i < num; i++)
	{
		if (iCount && sTmp.length() - iCount > 0)
			sTmp = input.substr(iCount, sTmp.length() - iCount);

		std::size_t pos = sTmp.find(iterator);

		if (pos == std::string::npos)
			return {};

		sTmp = sTmp.substr(0, pos);

		iCount += sTmp.length() + 1;
	}

	return sTmp;
}

std::string XorEncryptDecryptLoader(const std::string sInput)
{
	VM_START
	const char szKey[] = { 'v','z','8','o','y','4','I','E','K','e','t','i','7','d','j','2','E','v','c','m','V','D','q','G','Q','P','k','C','y','l','X','C' };

	std::string sOutput = sInput;

	for (size_t i = 0; i < sInput.size(); ++i)
		sOutput[i] = sInput[i] ^ szKey[i % (sizeof(szKey) / sizeof(char))];
	VM_END
	return sOutput;
}

std::string HexToString(const std::string& sInput)
{
	const char* const lut = "0123456789ABCDEF";
	const size_t len = sInput.length();

	if (len & 1)
		return sInput;

	std::string sOutPut;
	sOutPut.reserve(len / 2);

	for (size_t i = 0; i < len; i += 2)
	{
		const char a = sInput[i];
		const char* p = std::lower_bound(lut, lut + 16, a);

		if (*p != a)
			return sInput;

		const char b = sInput[i + 1];
		const char* q = std::lower_bound(lut, lut + 16, b);

		if (*q != b)
			return sInput;

		sOutPut.push_back(((p - lut) << 4) | (q - lut));
	}

	return sOutPut;
}

std::string StringToHex(const std::string& sInput)
{
	const char* const lut = "0123456789ABCDEF";
	const size_t len = sInput.length();

	std::string sOutput;
	sOutput.reserve(2 * len);

	for (size_t i = 0; i < len; ++i)
	{
		const unsigned char c = sInput[i];
		sOutput.push_back(lut[c >> 4]);
		sOutput.push_back(lut[c & 15]);
	}
	return sOutput;
}

std::string GetUrlData(const std::string& sURL)
{
	VM_START;
	g.m_dwDownloadedBytes = 0;
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
		{
			sRequestData.append(cBuff, dwRead);
			g.m_dwDownloadedBytes += dwRead;
		}
	}

	InternetCloseHandle(hHttpRequest);
	InternetCloseHandle(hHttpSession);
	InternetCloseHandle(hIntSession);
	VM_END;
	return sRequestData;
}

std::string GetHardwareID()
{
	VM_START;
	STR_ENCRYPTW_START;
	HRESULT hResult = CoInitializeEx(nullptr, COINIT_MULTITHREADED);

	if (FAILED(hResult))
		return "";

	IWbemLocator* pLocator = nullptr;
	hResult = CoCreateInstance(CLSID_WbemLocator, nullptr, CLSCTX_INPROC_SERVER, IID_IWbemLocator, reinterpret_cast<LPVOID*>(&pLocator));

	if (FAILED(hResult))
		return "";

	IWbemServices* pServices = nullptr;
	hResult = pLocator->ConnectServer(_bstr_t("ROOT\\CIMV2"), nullptr, nullptr, nullptr, 0, nullptr, nullptr, &pServices);

	if (FAILED(hResult))
		return "";

	hResult = CoSetProxyBlanket(pServices, RPC_C_AUTHN_WINNT, RPC_C_AUTHZ_NONE, NULL, RPC_C_AUTHN_LEVEL_CALL, RPC_C_IMP_LEVEL_IMPERSONATE, NULL, EOAC_NONE);

	if (FAILED(hResult))
		return "";

	IEnumWbemClassObject* pEnumerator = nullptr;
	hResult = pServices->ExecQuery(_bstr_t("WQL"), _bstr_t("SELECT * FROM Win32_BaseBoard"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

	if (FAILED(hResult))
		return "";

	std::wstringstream wssHardwareID;
	wssHardwareID << time(nullptr) << ';';
	wssHardwareID << GetCurrentProcessId() << ';';

	IWbemClassObject* pClassObj = nullptr;
	ULONG ulReturn = 0;

	while (pEnumerator)
	{
		pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

		if (!ulReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		pClassObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		pClassObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pClassObj->Release();
	}

	pEnumerator = nullptr;
	hResult = pServices->ExecQuery(_bstr_t("WQL"), _bstr_t("SELECT * FROM Win32_BIOS"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

	if (FAILED(hResult))
		return "";

	pClassObj = nullptr;
	ulReturn = 0;

	while (pEnumerator)
	{
		pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

		if (!ulReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		pClassObj->Get(L"SerialNumber", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		pClassObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		pClassObj->Get(L"Version", 0, &vtProp, 0, 0);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pClassObj->Release();
	}

	pEnumerator = nullptr;
	hResult = pServices->ExecQuery(_bstr_t("WQL"), _bstr_t("SELECT * FROM Win32_OperatingSystem"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

	if (FAILED(hResult))
		return "";

	pClassObj = nullptr;
	ulReturn = 0;

	while (pEnumerator)
	{
		pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

		if (!ulReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		pClassObj->Get(L"InstallDate", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);

		pClassObj->Get(L"Manufacturer", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pClassObj->Release();
	}

	pEnumerator = nullptr;
	hResult = pServices->ExecQuery(_bstr_t("WQL"), _bstr_t("SELECT * FROM Win32_ComputerSystemProduct"), WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY, nullptr, &pEnumerator);

	if (FAILED(hResult))
		return "";

	pClassObj = nullptr;
	ulReturn = 0;

	while (pEnumerator)
	{
		pEnumerator->Next(WBEM_INFINITE, 1, &pClassObj, &ulReturn);

		if (!ulReturn)
			break;

		VARIANT vtProp;
		STR_ENCRYPTW_START;
		pClassObj->Get(L"UUID", 0, &vtProp, nullptr, nullptr);
		wssHardwareID << vtProp.bstrVal << ';';
		VariantClear(&vtProp);
		STR_ENCRYPTW_END;
		pClassObj->Release();
	}

	pServices->Release();
	pLocator->Release();
	pEnumerator->Release();
	CoUninitialize();

	std::wstring wsWideHWID = wssHardwareID.str();
	wssHardwareID.clear();

	std::string sHashedHWID(wsWideHWID.begin(), wsWideHWID.end());
	wsWideHWID.clear();

	sHashedHWID = StringToHex(XorEncryptDecryptLoader(sHashedHWID));
	STR_ENCRYPTW_END;
	VM_END;
	return sHashedHWID;
}

std::string ReplaceAll(std::string str, const std::string& from, const std::string& to) 
{
	size_t start_pos = 0;

	while ((start_pos = str.find(from, start_pos)) != std::string::npos) 
	{
		str.replace(start_pos, from.length(), to);
		start_pos += to.length(); // Handles case where 'to' is a substring of 'from'
	}

	return str;
}

void GetBuilds()
{
	std::string sUrl = "/hpp/server.php?loader=1&type=1&hwid=" + g_LicenseInfo.m_sHardwareID;
	std::string sResponse = GetUrlData(sUrl);
	std::string sBuffer = sResponse;

	for (auto i = 1; i < SHRT_MAX; i++)
	{
		sBuffer = FindArgument(sResponse, i, "^");

		if (sBuffer.length() < 2)
			break;

		std::string sType = FindArgument(sBuffer, 1);
		
		if (sType.length() < 2)
			break;

		std::string sProduct = "hpp_cs16";

		if (sType.find(sProduct) != std::string::npos)
			g_Build.Add("Hpp Hack: Reload", "5.0 Beta", "Counter-Strike 1.6", "&type=3&product=1", "Hpp Hack Reload", "hl.exe", "Support:^- Valve Anti-Cheat^- Server Side Anti-Cheat^");
	}

	for (size_t i = 0; i < g_BuildInfo.size(); ++i)
		g_BuildsList.push_back(g_BuildInfo[i].m_sBuildName);

	if (g_BuildInfo.size())
	{
		g_LicenseInfo.m_bIsUserValid = true;
		g_LicenseInfo.m_bIsGetBuild = true;
	}
}