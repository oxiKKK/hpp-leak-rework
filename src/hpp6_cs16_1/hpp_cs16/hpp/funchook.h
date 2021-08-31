class CHookData {
	FARPROC func;
	FARPROC hookFunc;
	bool bHooked;
	BYTE data[14];

public:
	inline CHookData()
		: bHooked(false)
		, func(NULL)
		, hookFunc(NULL)
		, origFunc(NULL)
		, data()
	{
		MH_Initialize();
	}
	FARPROC origFunc;

	inline bool Hook(FARPROC funcIn, FARPROC hookFuncIn)
	{
		if (bHooked) {
			if (funcIn == func) {
				if (hookFunc != hookFuncIn) {
					hookFunc = hookFuncIn;
					Rehook();
					return true;
				}
			}

			Unhook();
		}

		func = funcIn;
		hookFunc = hookFuncIn;

		DWORD oldProtect;
		if (!VirtualProtect((LPVOID)func, 14, PAGE_EXECUTE_READWRITE, &oldProtect))
			return false;

		// FIXME: check 64 bit instructions too
		if (*(BYTE*)func == 0xE9 || *(BYTE*)func == 0xE8) {
			CHAR *modName, *ourName;
			CHAR szModName[MAX_PATH];
			CHAR szOurName[MAX_PATH];
			DWORD memAddress;

			MEMORY_BASIC_INFORMATION mem;

			INT_PTR jumpAddress = *(DWORD*)((BYTE*)func + 1) + (DWORD)func;

			// try to identify target
			if (VirtualQueryEx(GetCurrentProcess(), (LPVOID)jumpAddress, &mem, sizeof(mem)) && mem.State == MEM_COMMIT)
				memAddress = (DWORD)mem.AllocationBase;
			else
				memAddress = jumpAddress;

			if (GetMappedFileNameA(GetCurrentProcess(), (LPVOID)memAddress, szModName, _countof(szModName) - 1))
				modName = szModName;
			else if (GetModuleFileNameA((HMODULE)memAddress, szModName, _countof(szModName) - 1))
				modName = szModName;
			else
				modName = (char*)"unknown";

			// and ourselves
			if (VirtualQueryEx(GetCurrentProcess(), (LPVOID)func, &mem, sizeof(mem)) && mem.State == MEM_COMMIT)
				memAddress = (DWORD)mem.AllocationBase;
			else
				memAddress = (DWORD)func;

			if (GetMappedFileNameA(GetCurrentProcess(), (LPVOID)memAddress, szOurName, _countof(szOurName) - 1))
				ourName = szOurName;
			else if (GetModuleFileNameA((HMODULE)memAddress, szOurName, _countof(szOurName) - 1))
				ourName = szOurName;
			else
				ourName = (char*)"unknown";

			CHAR* p = strrchr(ourName, '\\');
			if (p)
				ourName = p + 1;

			if (g_pConsole)
				g_pConsole->DPrintf("WARNING: Another hook is already present while trying to hook %s, hook target is %s. If you experience crashes, try disabling the other hooking application.\n", ourName, modName);
		}

		memcpy(data, (const void*)func, 14);
		//VirtualProtect((LPVOID)func, 14, oldProtect, &oldProtect);

		return true;
	}

	inline void Rehook(bool bForce = false)
	{
		if ((!bForce && bHooked) || !func)
			return;

		MH_CreateHook(func, hookFunc, (void**)&origFunc);
		MH_EnableHook(func);

		bHooked = true;
	}

	inline void Unhook()
	{
		if (!bHooked || !func)
			return;

		MH_DisableHook(func);
		MH_RemoveHook(func);

		bHooked = false;
	}
};