class CHookData 
{
public:
	FARPROC m_OrigFunc;

	inline CHookData() : 
		m_bIsHooked(false),
		m_Func(nullptr), 
		m_HookFunc(nullptr), 
		m_OrigFunc(nullptr),
		m_data()
	{
		MH_Initialize();
	}

	inline bool IsHooked()
	{
		return m_bIsHooked;
	}

	inline bool Hook(FARPROC funcIn, FARPROC hookFuncIn)
	{
		if (m_bIsHooked) 
		{
			if (funcIn == m_Func && m_HookFunc != hookFuncIn)
			{
				m_HookFunc = hookFuncIn;
				Rehook();
				return true;
			}

			Unhook();
		}

		m_Func = funcIn;
		m_HookFunc = hookFuncIn;

		DWORD dwOldProtect;

		if (!VirtualProtect((LPVOID)m_Func, sizeof(m_data), PAGE_EXECUTE_READWRITE, &dwOldProtect))
			return false;

		memcpy(m_data, (const void*)m_Func, sizeof(m_data));

		return true;
	}

	inline void Rehook(bool bForce = false)
	{
		if ((!bForce && m_bIsHooked) || !m_Func)
			return;

		MH_CreateHook(m_Func, m_HookFunc, (void**)& m_OrigFunc);
		MH_EnableHook(m_Func);
	
		m_bIsHooked = true;
	}

	inline void Unhook()
	{
		if (!m_bIsHooked || !m_Func)
			return;

		MH_DisableHook(m_Func);
		MH_RemoveHook(m_Func);

		m_bIsHooked = false;
	}

private:
	FARPROC m_Func;
	FARPROC m_HookFunc;
	bool m_bIsHooked;
	BYTE m_data[14];
};

#define MAKE_HOOK(hook, address, function)\
	hook.Hook((FARPROC)address, (FARPROC)function);\
	hook.Rehook();

#define RESET_HOOK(hook) hook.Unhook();