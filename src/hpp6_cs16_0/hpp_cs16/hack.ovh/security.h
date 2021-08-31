typedef NTSTATUS(NTAPI* pNtSetInformationThread)(HANDLE, UINT, PVOID, ULONG);

bool HideThread(HANDLE hThread);
std::string GetUrlData(const std::string& sURL);
void LicenseCheck();

