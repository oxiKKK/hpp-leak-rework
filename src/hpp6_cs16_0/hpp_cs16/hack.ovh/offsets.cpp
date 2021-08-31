#include "main.h"

COffsets g_Offsets;

bool COffsets::FindHardware()
{
	if (!FindModuleByName("hw.dll", &hw))
		return false;

	if (!FindModuleByName("client.dll", &client))
		return false;

	if (!FindModuleByName("gameui.dll", &gameui))
		return false;

	if (!FindModuleByName("vgui2.dll", &vgui2))
		return false;

	if (!FindModuleByName("gameoverlayrenderer.dll", &gor))
		TraceLog("> %s: module gameoverlayrenderer.dll not found.\n", __FUNCTION__);

	return true;
}

bool COffsets::FindOffsets(void)
{
	if (!(g_pClient = (cl_clientfunc_t*)FindClient()))
		return false;

	if (!(g_pEngine = (cl_enginefunc_t*)FindEngine()))
		return false;

	if (!(g_pStudio = (engine_studio_api_t*)FindStudio()))
		return false;

	if (!(g_pStudioModelRenderer = (StudioModelRenderer_t*)FindStudioRenderModel()))
		return false;

	if (!g_Client.V_CalcRefdef)
		RtlCopyMemory(&g_Client, g_pClient, sizeof(cl_clientfunc_t));

	if (!g_Engine.V_CalcShake)
		RtlCopyMemory(&g_Engine, g_pEngine, sizeof(cl_enginefunc_t));

	if (!g_Studio.StudioSetupSkin)
		RtlCopyMemory(&g_Studio, g_pStudio, sizeof(engine_studio_api_t));

	if (!g_StudioModelRenderer.StudioSlerpBones)
		RtlCopyMemory(&g_StudioModelRenderer, g_pStudioModelRenderer, sizeof(StudioModelRenderer_t));

	if (!g_Utils.EnablePageWrite((DWORD)g_pStudioModelRenderer, sizeof(StudioModelRenderer_t)))
		return false;

	if (!(pmove = (playermove_t*)FindPlayerMove()))
		return false;

	if (!(g_pPreS_DynamicSound = (PreS_DynamicSound_t)PreS_DynamicSound()))
		return false;

	auto pg_pClientUserMsgs = FindClientUserMsgs();

	if (!pg_pClientUserMsgs)
		return false;

	g_pClientUserMsgs = (UserMsg)pg_pClientUserMsgs;

	if (!FindSendCommand())
		return false;

	if (!(client_state = (client_state_t*)FindClientState()))
		return false;

	if (!(client_static = (client_static_t*)FindClientStatic()))
		return false;

	//if (!(g_Globals.allow_cheats = (bool*)AllowCheats()))
	//	return false;

	//	oxiKKK
	if (!(g_Globals.allow_cheats = (bool*)true))
		return false;

	if (!(g_PlayerExtraInfo = (extra_player_info_t*)FindExtraPlayerInfo()))
		return false;

	if (!(g_pEngineMessages = (svc_func_t*)FindEngineMessages()))
		return false;

	if (!(g_pR_DrawEntitiesOnList = (R_DrawEntitiesOnList_t)R_DrawEntitiesOnList()))
		return false;

	return true;
}

DWORD COffsets::R_DrawEntitiesOnList()
{
	DWORD dwAddress = FindPush((PCHAR)"progs/flame.mdl", hw.base, hw.end);

	dwAddress = FindPattern((PCHAR)"\x90\xD9\x05", (PCHAR)"xxx", dwAddress, hw.end, 1); //R_DrawEntitiesOnList

	if (FarProc(dwAddress, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);

	DWORD dwAddress2 = FindPattern((PCHAR)"\x8B\x0D", dwAddress, hw.end); //cl_numvisedicts

	if (FarProc(dwAddress2, hw.base, hw.end))
		Error("%s: cl_numvisedicts not found.", __FUNCTION__);

	g_Globals.cl_numvisedicts = (int*)*(DWORD*)(dwAddress2 + 2);

	DWORD dwAddress3 = FindPattern((PCHAR)"\x8B\x14", dwAddress2, dwAddress2 + 0x18); //cl_visedicts steam

	if (FarProc(dwAddress3, hw.base, hw.end))
	{
		dwAddress3 = FindPattern((PCHAR)"\x55\xBE", dwAddress2, dwAddress2 + 0x18, 1); //cl_visedicts

		g_Globals.cl_visedicts = (cl_entity_t**)*(DWORD*)(dwAddress3 + 1);

		TraceLog("> %s: find cl_visedicts old version.\n", __FUNCTION__);
	}
	else
		g_Globals.cl_visedicts = (cl_entity_t**)*(DWORD*)(dwAddress3 + 3);

	return dwAddress;
}

PVOID COffsets::FindEngineMessages(void)
{
	DWORD dwEngineMsgBase = FindPattern((PCHAR)"\xBF\xFF\xFF\xFF\xFF\x8B\x04\xB5\xFF\xFF\xFF\xFF\x85\xC0\x74\xFF\x81\xFF\xFF\xFF\xFF\xFF\x7F\x04\x8B\x0F\xEB\x05", (PCHAR)"x????xxx????xxx?xx????xxxxxx", hw.base, hw.end, 1);

	auto pEngineMsgBase = (svc_func_t*)(*(PDWORD)dwEngineMsgBase - sizeof(DWORD));

	if (pEngineMsgBase)
	{
		DWORD SVC_SoundBase = (DWORD)pEngineMsgBase[svc_sound].pfnParse;

		if (*(PBYTE)(SVC_SoundBase + 0x0E) == 0xE8)
			pMSG_Buffer = (sizebuf_t *)(*(PDWORD)(SVC_SoundBase + 0x0A));
		else if (*(PBYTE)(SVC_SoundBase + 0x0C) == 0xE8)
			pMSG_Buffer = (sizebuf_t *)(*(PDWORD)(SVC_SoundBase + 0x08));
		else
			Error("%s: pMSG_Buffer not found.", __FUNCTION__);
	}
	else
		Error("%s: not found.", __FUNCTION__);

	return (PVOID)pEngineMsgBase;
}

DWORD COffsets::FindExtraPlayerInfo(void)
{
	DWORD dwAddress = (DWORD)g_pClient->HUD_GETPLAYERTEAM_FUNCTION;

	dwAddress = FindPattern((PCHAR)"\x0F\xBF\x04\xFF\xFF\xFF\xFF\xFF\xC3", (PCHAR)"xxx?????x", dwAddress - 30, dwAddress);

	dwAddress = *(DWORD*)(dwAddress + 4);

	dwAddress = (dwAddress - offsetof(extra_player_info_t, team_id));

	if (FarProc(dwAddress, client.base, client.end))
		Error("%s: not found.", __FUNCTION__);

	return dwAddress;
}

DWORD COffsets::AllowCheats(void)
{
	//	oxiKKK
	//auto ptr = FindPush((PCHAR)"Server must enable cheats to activate fakelag", hw.base, hw.end, -0x22);
	//
	//ptr = *(uintptr_t*)(ptr + 1);
	//
	//if (FarProc(ptr, hw.base, hw.end))
	//{
	//	Error("%s: not found.\n", __FUNCTION__);
	//}

	return 0x0;
}

PVOID COffsets::FindClientStatic(void)
{
	DWORD dwAddress = FindPush((PCHAR) "in :  %i %.2f k/s", hw.base, hw.end);
	
	dwAddress = FindPattern((PCHAR) "\xA1\xFF\xFF\xFF\xFF\x83\xEC\x08", (PCHAR) "x????xxx", dwAddress - 0x30, dwAddress);

	dwAddress = *(DWORD*)(dwAddress + 1);

	dwAddress = (DWORD)(client_static_t*)(dwAddress - offsetof(client_static_t, netchan.incoming_sequence));

	if (FarProc(dwAddress, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);

	return (PVOID)dwAddress;
}

DWORD COffsets::PreS_DynamicSound(void)
{
	DWORD dwAddress = Absolute(FindPush((PCHAR) "CL_Parse_Sound: ent = %i, cl.max_edicts %i", hw.base, hw.end) - 0x10);

	if (FarProc(dwAddress, hw.base, hw.end)) 
	{
		dwAddress = Absolute(FindPush((PCHAR) "CL_Parse_Sound: ent = %i, cl.max_edicts %i", hw.base, hw.end) - 0x11);

		if (FarProc(dwAddress, hw.base, hw.end))
			Error("%s: not found.", __FUNCTION__);
	}

	return dwAddress;
}

PVOID COffsets::FindClientUserMsgs(void)
{
	DWORD dwAddress = (DWORD)g_Engine.pfnHookUserMsg;
	DWORD dwUserMsgBase = Absolute(FindPattern((PCHAR) "\x52\x50\xE8\xFF\xFF\xFF\xFF\x83", (PCHAR) "xxx????x", dwAddress, dwAddress + 0x32, 0x3));

	dwUserMsgBase = FindPattern((PCHAR) "\xFF\xFF\xFF\x0C\x56\x8B\x35\xFF\xFF\xFF\xFF\x57", (PCHAR) "???xxxx????x", dwUserMsgBase, dwUserMsgBase + 0x32, 0x7);

	if (FarProc(dwUserMsgBase, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);

	return (PVOID)UserMsg(**(PDWORD*)dwUserMsgBase);
}

PVOID COffsets::FindPlayerMove(void)
{
#ifdef LICENSE
	VM_START;
	DWORD dwAddress = FindPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	PVOID pvPointer = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, dwAddress) + m_dwCloudOffsets[1]);

	if (FarProc((DWORD)pvPointer, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);
	VM_END;
#else
	DWORD dwAddress = FindPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	PVOID pvPointer = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, dwAddress) + 0x18);

	if (FarProc((DWORD)pvPointer, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);
#endif // LICENSE

	return pvPointer;
}

PVOID COffsets::FindStudioRenderModel(void)
{
	DWORD dwAddress = FindPattern((PCHAR) "\x56\x8B\xF1\xE8\xFF\xFF\xFF\xFF\xC7\x06\xFF\xFF\xFF\xFF\xC6\x86\xFF\xFF\xFF\xFF\xFF\x8B\xC6\x5E\xC3",
		(PCHAR) "xxxx????xx????xx?????xxxx", client.base, client.end);

	if (FarProc((DWORD)dwAddress, client.base, client.end))
		Error("%s: not found.", __FUNCTION__);

	return (PVOID) * (DWORD*)(dwAddress + 0x0A);
}

PVOID COffsets::FindStudio(void)
{
	PVOID pvPointer = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x30);

	if (FarProc((DWORD)pvPointer, hw.base, hw.end))
	{
		pvPointer = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x1A);

		if (FarProc((DWORD)pvPointer, client.base, client.end))
			Error("%s: not found.", __FUNCTION__);
	}

	return pvPointer;
}

PVOID COffsets::FindEngine(void)
{
#ifdef LICENSE
	VM_START;
	SYSTEMTIME st;
	GetSystemTime(&st);

	int fix = st.wMonth - 2;

	if (st.wMonth == 10)
		fix--;

	DWORD dwAddress = *(PDWORD)(FindPush((PCHAR) "sprites/voiceicon.spr", client.base, client.end) + fix);

	if (FarProc(dwAddress, client.base, client.end))
		Error("%s: not found.", __FUNCTION__);
	VM_END;
#else
	DWORD dwAddress = *(PDWORD)(FindPush((PCHAR) "sprites/voiceicon.spr", client.base, client.end) + 0x07);

	if (FarProc(dwAddress, client.base, client.end))
		Error("%s: not found.", __FUNCTION__);
#endif // LICENSE
	return (PVOID)dwAddress;
}

PVOID COffsets::FindClient(void)
{
#ifdef LICENSE
	VM_START;
	DWORD dwAddress = FindPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	PVOID pvPointer = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, dwAddress) + m_dwCloudOffsets[0]); // 0x13 for cloud

	if (FarProc((DWORD)pvPointer, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);
	VM_END;
#else
	DWORD dwAddress = FindPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	PVOID pvPointer = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, dwAddress) + 0x13); // 0x13 for cloud

	if (FarProc((DWORD)pvPointer, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);
#endif // LICENSE

	return pvPointer;
}

bool COffsets::FindSendCommand(void)
{
	m_dwSendCommandPointer1 = FindPattern((PCHAR)"\x83\xC4\x08\x39\x3D", (PCHAR)"xxxxx", hw.base, hw.end, (DWORD)-0x11);

	if (FarProc(m_dwSendCommandPointer1, hw.base, hw.end))
	{
		TraceLog("> %s: finding already injected function.\n", __FUNCTION__);

		m_dwSendCommandPointer1 = FindPattern((PCHAR) "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", hw.base, hw.end);

		if (FarProc(m_dwSendCommandPointer1, hw.base, hw.end)) 
		{
			Error("%s: not found.", __FUNCTION__);
			return false;
		}
	}
	
	g_Utils.memwrite(m_dwSendCommandPointer1, (uintptr_t) "\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90\x90", 26);

	m_dwSendCommandPointer2 = FindPattern((PCHAR) "\x23\xC1\x41", (PCHAR) "xxx", m_dwSendCommandPointer1, hw.end, (DWORD)-0xB);

	if (FarProc(m_dwSendCommandPointer2, hw.base, hw.end))
	{
		Error("%s: #2 not found.", __FUNCTION__);
		return false;
	}

	return true;
}

PVOID COffsets::FindClientState(void)
{
	DWORD dwAddress = FindPattern((PCHAR)"\x8D\x34\x95\xFF\xFF\xFF\xFF\x56\xC7\x46\x38\x00\x00\x80\xBF\x89\x7E\x40\x89\x7E\x44\x89\x7E\x48",
		(PCHAR)"xxx????xxxxxxxxxxxxxxxxx", hw.base, hw.end);

	dwAddress = *(DWORD*)(dwAddress + 3);

	if (FarProc(dwAddress, hw.base, hw.end))
		Error("%s: not found.", __FUNCTION__);

	dwAddress = (DWORD)(client_state_t*)(dwAddress - offsetof(client_state_t, commands));

	return (PVOID)dwAddress;
}

DWORD COffsets::FindSteamOverlay()
{
	if (!gor.base || !gor.end)
		return NULL;

	DWORD dwAddress = FindPattern((PCHAR)"\xFF\x15\xFF\xFF\xFF\xFF\x80\x3D\xFF\xFF\xFF\xFF\xFF\x8B\xF0\x74\x0C\x68\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x8B\xC6\x5E\x8B\xE5\x5D\xC2\x04\x00",
		(PCHAR)"xx????xx?????xxxxx????x????xxxxxxxxx", gor.base, gor.end, 0x02);

	if (dwAddress)
		dwAddress = *(DWORD*)dwAddress;

	if (dwAddress)
		dwAddress = *(DWORD*)dwAddress;

	return dwAddress;
}

DWORD COffsets::Absolute(DWORD dwAddress)
{
	return dwAddress + *(PDWORD)dwAddress + 0x4;
}

DWORD COffsets::FindReference(DWORD dwStart, DWORD dwEnd, DWORD dwAddress)
{
	char szPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	*(PDWORD)&szPattern[1] = dwAddress;
	return FindPattern(szPattern, dwStart, dwEnd, 0);
}

DWORD COffsets::FindPush(PCHAR pszMessage, DWORD dwStart, DWORD dwEnd, DWORD dwOffset)
{
	char bPushAddrPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	DWORD Address = FindPattern(pszMessage, dwStart, dwEnd, 0);
	*(PDWORD)&bPushAddrPattern[1] = Address;
	Address = FindPattern((PCHAR)bPushAddrPattern, dwStart, dwEnd, 0);
	return Address + dwOffset;
}

DWORD COffsets::FarProc(const DWORD dwAddress, DWORD LB, DWORD HB)
{
	return ((dwAddress < LB) || (dwAddress > HB));
}

void COffsets::Error(const char *fmt, ...)
{
	//VM_START
	STR_ENCRYPT_START
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	vsnprintf(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	MessageBox(0, string, "Hpp Hack", MB_OK | MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), 0);
	STR_ENCRYPT_END
	//VM_END
}

bool COffsets::FindModuleByName(const char* moduleName, Module* module)
{
	if (!moduleName || !*moduleName || !module)
		return false;

	HMODULE hModuleDll = GetModuleHandle(moduleName);

	if (hModuleDll == INVALID_HANDLE_VALUE)
		return false;

	MEMORY_BASIC_INFORMATION mem;

	if (!VirtualQuery(hModuleDll, &mem, sizeof(mem)))
		return false;

	if (mem.State != MEM_COMMIT)
		return false;

	if (!mem.AllocationBase)
		return false;

	IMAGE_DOS_HEADER *dos = (IMAGE_DOS_HEADER *)mem.AllocationBase;
	IMAGE_NT_HEADERS *pe = (IMAGE_NT_HEADERS *)((uintptr_t)dos + (uintptr_t)dos->e_lfanew);

	if (pe->Signature != IMAGE_NT_SIGNATURE)
		return false;

	module->base = (DWORD)mem.AllocationBase;
	module->size = (size_t)pe->OptionalHeader.SizeOfImage;
	module->end = module->base + module->size - 1;

	return true;
}

DWORD COffsets::FindPattern(PCHAR pszPattern, PCHAR pszMask, DWORD dwStart, DWORD dwEnd, DWORD dwOffset)
{
	if (!dwStart || !dwEnd)
		return 0;

	if (dwStart > dwEnd) 
	{
		DWORD dwReverse = dwEnd;
		dwEnd = dwStart;
		dwStart = dwReverse;
	}

	size_t patternLength = strlen(pszPattern);
	bool found = false;

	for (DWORD i = dwStart; i < dwEnd - patternLength; ++i)
	{
		found = true;

		for (size_t idx = 0; idx < patternLength; ++idx) 
		{
			if (pszMask[idx] == 'x' && pszPattern[idx] != *(PCHAR)(i + idx)) {
				found = false;
				break;
			}
		}

		if (found)
			return i + dwOffset;
	}

	return 0;
}

DWORD COffsets::FindPattern(PCHAR pszPattern, DWORD dwStart, DWORD dwEnd, DWORD dwOffset)
{
	if (!dwStart || !dwEnd)
		return 0;

	if (dwStart > dwEnd) 
	{
		DWORD dwReverse = dwEnd;
		dwEnd = dwStart;
		dwStart = dwReverse;
	}

	size_t patternLength = strlen(pszPattern);
	bool found = false;

	for (DWORD i = dwStart; i < dwEnd - patternLength; ++i) 
	{
		found = true;

		for (size_t idx = 0; idx < patternLength; ++idx)
		{
			if (pszPattern[idx] != *(PCHAR)(i + idx)) {
				found = false;
				break;
			}
		}

		if (found)
			return i + dwOffset;
	}

	return 0;
}