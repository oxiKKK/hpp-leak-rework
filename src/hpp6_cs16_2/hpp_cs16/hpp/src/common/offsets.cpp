#include "framework.h"

PreS_DynamicSound_t g_pPreS_DynamicSound;
CL_DriftInterpolationAmount_t g_pCL_DriftInterpolationAmount;
CL_RecordHUDCommand_t g_pCL_RecordHUDCommand;
R_DrawEntitiesOnList_t g_pR_DrawEntitiesOnList;
InitiateGameConnection_t g_pInitiateGameConnection;
IN_MouseMove_t g_pIN_MouseMove;
NET_SendPacket_t g_pNET_SendPacket;
CL_RunUsercmd_t g_pCL_RunUsercmd;
Host_FilterTime_t g_pHost_FilterTime;
SCR_UpdateScreen_t g_pSCR_UpdateScreen;
PClientUserMsg g_pClientUserMsgs;

bool COffsets::FindModules()
{
	if (!m_bModulesFound)
	{
		if (!FindModuleByName("hw.dll", &Module[HW]))
			return false;

		if (!FindModuleByName("client.dll", &Module[CLIENT]))
			return false;

		if (!FindModuleByName("gameui.dll", &Module[GAMEUI]))
			return false;

		if (!FindModuleByName("vgui2.dll", &Module[VGUI2]))
			return false;

		m_bModulesFound = true;

		if (!FindModuleByName(V("gameoverlayrenderer.dll"), &Module[GAMEOVERLAYRENDERER]))
			Utils::TraceLog(V("> %s: module gameoverlayrenderer.dll not found.\n"), V(__FUNCTION__));

		if (!FindModuleByName(V("steamclient.dll"), &Module[STEAMCLIENT]))
			Utils::TraceLog(V("> %s: module steamclient.dll not found.\n"), V(__FUNCTION__));
	}

	return true;
}

bool COffsets::FindInterfaces()
{
	if (!m_bInterfacesFound)
	{
		CreateInterfaceFn Factory[MODULES_MAX_COUNT];
		Factory[HW] = CaptureFactory(Module[HW].handle);
		Factory[GAMEUI] = CaptureFactory(Module[GAMEUI].handle);
		Factory[STEAMCLIENT] = CaptureFactory(Module[STEAMCLIENT].handle);
		Factory[VGUI2] = CaptureFactory(Module[VGUI2].handle);

		if (!Factory[HW] || !Factory[GAMEUI] || !Factory[VGUI2])
			return false;

		g_pISurface = (ISurface*)(CaptureInterface(Factory[HW], VGUI_SURFACE_INTERFACE_VERSION));
		g_pIGameUI = (IGameUI*)(CaptureInterface(Factory[GAMEUI], GAMEUI_INTERFACE_VERSION));
		g_pIGameUIFuncs = (IGameUIFuncs*)(CaptureInterface(Factory[HW], VENGINE_GAMEUIFUNCS_VERSION));
		g_pIRunGameEngine = (IRunGameEngine*)(CaptureInterface(Factory[GAMEUI], RUNGAMEENGINE_INTERFACE_VERSION));
		g_pConsole = (IGameConsole*)(CaptureInterface(Factory[GAMEUI], GAMECONSOLE_INTERFACE_VERSION));
		g_pIEngineVGui = (IEngineVGui*)(CaptureInterface(Factory[HW], VENGINE_VGUI_VERSION));
		g_pIPanel = (IPanel*)(CaptureInterface(Factory[VGUI2], VGUI_PANEL_INTERFACE_VERSION));

		if (!g_pISurface || !g_pIGameUI || !g_pIGameUIFuncs || !g_pIRunGameEngine || !g_pConsole || !g_pIEngineVGui || !g_pIPanel)
			return false;

		m_bInterfacesFound = true;

		auto FindSteamInterfaces = [](CreateInterfaceFn Interface)
		{
			if (!Interface)
			{
				Utils::TraceLog(V("%s: factory steamclient not found.\n"), V(__FUNCTION__));
				return;
			}

			g_pISteamClient = (ISteamClient*)(CaptureInterface(Interface, STEAMCLIENT_INTERFACE_VERSION));

			if (!g_pISteamClient)
			{
				Utils::TraceLog(V("%s: interface steamclient not found.\n"), V(__FUNCTION__));
				return;
			}

			HSteamPipe hSteamPipe = g_pISteamClient->CreateSteamPipe();

			if (!hSteamPipe)
			{
				Utils::TraceLog(V("%s: steampipe not found.\n"), V(__FUNCTION__));
				return;
			}

			HSteamUser hSteamUser = g_pISteamClient->ConnectToGlobalUser(hSteamPipe);

			if (!hSteamUser)
			{
				g_pISteamClient->BReleaseSteamPipe(hSteamPipe);
				g_pISteamClient->BShutdownIfAllPipesClosed();

				Utils::TraceLog(V("%s: steamuser not found.\n"), V(__FUNCTION__));
				return;
			}

			g_pISteamScreenshots = g_pISteamClient->GetISteamScreenshots(hSteamUser, hSteamPipe, STEAMSCREENSHOTS_INTERFACE_VERSION);
			g_pISteamUtils = g_pISteamClient->GetISteamUtils(hSteamPipe, STEAMUTILS_INTERFACE_VERSION);
			g_pISteamFriends = g_pISteamClient->GetISteamFriends(hSteamUser, hSteamPipe, STEAMFRIENDS_INTERFACE_VERSION);
			g_pISteamUser = g_pISteamClient->GetISteamUser(hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION);
		};

		FindSteamInterfaces(Factory[STEAMCLIENT]);
	}

	return true;
}

bool COffsets::FindGameOffsets()
{
	if (!(g_pClient = (cl_clientfunc_t*)(Client())))
		return false;

	if (!(g_pEngine = (cl_enginefunc_t*)(Engine())))
		return false;

	if (!(g_pStudio = (engine_studio_api_t*)(Studio())))
		return false;

	if (!(g_pStudioAPI = (r_studio_interface_t*)(StudioAPI())))
		return false;

	if (!(g_pStudioModelRenderer = (StudioModelRenderer_t*)(StudioModelRenderer())))
		return false;

	if (!(pmove = (playermove_t*)(PlayerMove())))
		return false;

	if (!(g_pPreS_DynamicSound = (PreS_DynamicSound_t)PreS_DynamicSound()))
		return false;

	if (!(g_pClientUserMsgs = (PClientUserMsg)(ClientUserMsgs())))
		return false;

	if (!(g_pClientEngineMsgs = (svc_func_t*)(EngineMsgs())))
		return false;

	if (!(client_state = (client_state_t*)(ClientState())))
		return false;

	if (!(client_static = (client_static_t*)(ClientStatic())))
		return false;

	if (!(g_pCL_DriftInterpolationAmount = (CL_DriftInterpolationAmount_t)CL_DriftInterpolationAmount()))
		return false;

	if (!(g_pCL_RecordHUDCommand = (CL_RecordHUDCommand_t)CL_RecordHUDCommand()))
		return false;

	if (!(g_pR_DrawEntitiesOnList = (R_DrawEntitiesOnList_t)R_DrawEntitiesOnList()))
		return false;
	
	if (!(g_pInitiateGameConnection = (InitiateGameConnection_t)InitiateGameConnection()))
		return false;

	/*if (!(g_pIN_MouseMove = (IN_MouseMove_t)IN_MouseMove()))
		return false;*/

	/*if(!(g_pNET_SendPacket = (NET_SendPacket_t)NET_SendPacket()))
		return false;*/

	/*if (!(g_pCL_RunUsercmd = (CL_RunUsercmd_t)CL_RunUsercmd()))
		return false;*/

	/*if (!(g_pHost_FilterTime = (Host_FilterTime_t)Host_FilterTime()))
		return false;

	if (!(g_pSCR_UpdateScreen = (SCR_UpdateScreen_t)SCR_UpdateScreen()))
		return false;*/

	if (!(g_PlayerExtraInfo.GetAddress(PlayerExtraInfo())))
		return false;

	if (!(g_pGlobals->m_iGameBuild = GetGameBuild()))
		return false;

	if (!(g_pGlobals->m_dwInitPoint = (uintptr_t)InitPoint()))
		return false;

	g_pGlobals->m_bAllowCheats = (bool*)AllowCheats();

#if !defined(LICENSING) || defined(DEBUG)
	g_pConsole->DPrintf("> %s: g_pClient: 0x%X\n", __FUNCTION__, g_pClient);
	g_pConsole->DPrintf("> %s: g_pEngine: 0x%X\n", __FUNCTION__, g_pEngine);
	g_pConsole->DPrintf("> %s: g_pStudio: 0x%X\n", __FUNCTION__, g_pStudio);
	g_pConsole->DPrintf("> %s: g_pStudioAPI: 0x%X\n", __FUNCTION__, g_pStudioAPI);
	g_pConsole->DPrintf("> %s: pmove: 0x%X\n", __FUNCTION__, pmove);
	g_pConsole->DPrintf("> %s: g_pPreS_DynamicSound: 0x%X\n", __FUNCTION__, g_pPreS_DynamicSound);
	g_pConsole->DPrintf("> %s: g_pClientUserMsgs: 0x%X\n", __FUNCTION__, g_pClientUserMsgs);
	g_pConsole->DPrintf("> %s: client_state: 0x%X\n", __FUNCTION__, client_state);
	g_pConsole->DPrintf("> %s: client_static: 0x%X\n", __FUNCTION__, client_static);
	g_pConsole->DPrintf("> %s: g_pIN_MouseMove: 0x%X\n", __FUNCTION__, g_pIN_MouseMove);
	g_pConsole->DPrintf("> %s: g_pCL_DriftInterpolationAmount: 0x%X\n", __FUNCTION__, g_pCL_DriftInterpolationAmount);
	g_pConsole->DPrintf("> %s: g_pCL_RecordHUDCommand: 0x%X\n", __FUNCTION__, g_pCL_RecordHUDCommand);
	g_pConsole->DPrintf("> %s: g_pHost_FilterTime: 0x%X\n", __FUNCTION__, g_pHost_FilterTime);
	g_pConsole->DPrintf("> %s: g_pSCR_UpdateScreen: 0x%X\n", __FUNCTION__, g_pSCR_UpdateScreen);
	g_pConsole->DPrintf("> %s: g_pStudioModelRenderer: 0x%X\n", __FUNCTION__, g_pStudioModelRenderer);
	g_pConsole->DPrintf("> %s: g_pR_DrawEntitiesOnList: 0x%X\n", __FUNCTION__, g_pR_DrawEntitiesOnList);
	g_pConsole->DPrintf("> %s: g_pInitiateGameConnection: 0x%X\n", __FUNCTION__, g_pInitiateGameConnection);
	g_pConsole->DPrintf("> %s: g_pCL_RunUsercmd: 0x%X\n", __FUNCTION__, g_pCL_RunUsercmd);
	g_pConsole->DPrintf("> %s: g_pNET_SendPacket: 0x%X\n", __FUNCTION__, g_pNET_SendPacket);
	g_pConsole->DPrintf("> %s: g_pGlobals->m_iGameBuild: %i\n", __FUNCTION__, g_pGlobals->m_iGameBuild);
	g_pConsole->DPrintf("> %s: g_pGlobals->m_dwInitPoint: 0x%X\n", __FUNCTION__, g_pGlobals->m_dwInitPoint);
#endif
	RtlCopyMemory(&g_Client, g_pClient, sizeof(g_Client));
	RtlCopyMemory(&g_Engine, g_pEngine, sizeof(g_Engine));
	RtlCopyMemory(&g_Studio, g_pStudio, sizeof(g_Studio));
	RtlCopyMemory(&g_StudioAPI, g_pStudioAPI, sizeof(g_StudioAPI));
	RtlCopyMemory(&g_StudioModelRenderer, g_pStudioModelRenderer, sizeof(g_StudioModelRenderer));

	DWORD dwOldProtect;

	if (!VirtualProtect((LPVOID)(g_pStudioModelRenderer), sizeof(g_StudioModelRenderer), PAGE_EXECUTE_READWRITE, &dwOldProtect))
	{
		Utils::TraceLog(V("%s: g_pStudioModelRenderer memory protected.\n"), V(__FUNCTION__));
		return false;
	}

	return true;
}

PVOID COffsets::AllowCheats()
{
	auto ptr = FindDownPattern((PCHAR)V("\x7B\x22\xD9\x05"), Module[HW].base, Module[HW].end, 0x2);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = *(PDWORD)(ptr + 2);

	return (PVOID)ptr;
}

PVOID COffsets::IN_MouseMove()
{
	auto ptr = FindDownPattern((PCHAR)V("\x83\xEC\x1C\x8D\x44\x24\x10"), Module[CLIENT].base, Module[CLIENT].end);

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

PVOID COffsets::NET_SendPacket()
{
	auto ptr = FindPush((PCHAR)V("NET_SendPacket: bad address type"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = FindUpPattern((PCHAR)V("\x90\x90\x90"), ptr, Module[HW].base);

	while (*(PBYTE)ptr == 0x90) //Find start function
		ptr++;

	return (PVOID)ptr;
}

PVOID COffsets::CL_RunUsercmd()
{
	auto ptr = FindDownPattern((PCHAR)V("\x50\x51\xE8\x80\xFF\xFF\xFF"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = FindUpPattern((PCHAR)V("\x90\x90\x90"), ptr, Module[HW].base);

	while (*(PBYTE)ptr == 0x90) //Find start function
		ptr++;

	return (PVOID)ptr;
}

PVOID COffsets::StudioAPI()
{
	auto ptr = FindDownPattern((PCHAR)V("\xC7\xFF\xFF\xFF\xFF\xFF\xF3\xA5\xB9\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x5F\xB8\xFF\xFF\xFF\xFF\x5E\xC3"), (PCHAR)V("x?????xxx????x????xx????xx"), Module[CLIENT].base, Module[CLIENT].end, 0x2);

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)*(r_studio_interface_t**)ptr;
}

PVOID COffsets::InitiateGameConnection()
{
	auto ptr = FindPush((PCHAR)"%c%c%c%cconnect %i %i \"%s\" \"%s\"\n", Module[HW].base, Module[HW].end);

	ptr = FindUpPattern((PCHAR)V("\xE8\xFF\xFF\xFF\xFF\x83\xC4\x1C\x8B\xF8"), (PCHAR)V("x????xxxxx"), ptr, Module[HW].base);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = (intptr_t)(ptr + 5) + *(intptr_t*)(ptr + 1);

	return (PVOID)ptr;
}

PVOID COffsets::R_DrawEntitiesOnList()
{
	auto ptr = FindPush((PCHAR)"progs/flame2.mdl", Module[HW].base, Module[HW].end);

	ptr = FindDownPattern((PCHAR)V("\x90\x90\x90"), ptr, Module[HW].end);

	while (*(PBYTE)ptr == 0x90) //Find start next function
		ptr++;

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}
	
	auto ptr_cl_num_visedicts = FindDownPattern((PCHAR)V("\x8B\x0D"), ptr, Module[HW].end); //cl_numvisedicts

	if (AddressNotInSpace(ptr_cl_num_visedicts, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: cl_num_visedicts not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	g_pGlobals->m_pNumVisibleEntities = (int*) * (PDWORD)(ptr_cl_num_visedicts + 2);
	
	auto ptr_cl_visedicts = FindDownPattern((PCHAR)V("\x8B\x14"), ptr, Module[HW].end); //cl_visedicts

	if (AddressNotInSpace(ptr_cl_num_visedicts, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: cl_visedicts not found.\n"), V(__FUNCTION__));
		return NULL;
	}
	
	g_pGlobals->m_pVisibleEntities = (cl_entity_t * *) * (PDWORD)(ptr_cl_visedicts + 3);

	auto ptr_currententity = FindDownPattern((PCHAR)V("\x89\x15"), ptr, Module[HW].end);

	if (AddressNotInSpace(ptr_currententity, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: ptr_currententity not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	g_pGlobals->m_dwCurrentEntity = *(PDWORD)(ptr_currententity + 2);
	
	return (PVOID)ptr;
}

PVOID COffsets::InitPoint()
{
	auto ptr = FindDownPattern((PCHAR)V("\x8B\x44\x24\x04\x48\x83\xF8\x1D"), Module[CLIENT].base, Module[CLIENT].end);

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

int COffsets::GetGameBuild()
{
	auto cmd = Game::CommandByName("version");

	if (cmd)
	{		
		auto ptr = Absolute((uintptr_t)cmd->function + 0x17);

		if (ptr)
		{
			using function_t = int(__cdecl*)();

			return function_t(ptr)();
		}
	}

	Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));

	return NULL;
}

PVOID COffsets::StudioModelRenderer()
{
	auto ptr = *(PDWORD)(FindDownPattern((PCHAR)V("\x56\x8B\xF1\xE8\xFF\xFF\xFF\xFF\xC7\x06"), (PCHAR)V("xxxx????xx"), Module[CLIENT].base, Module[CLIENT].end, 0x0A)); // .text:019477A8                 mov     dword ptr [esi], offset ??_7CGameStudioModelRenderer@@6B@ ; const CGameStudioModelRenderer::`vftable'

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

PVOID COffsets::PlayerExtraInfo()
{
	auto start = (uintptr_t)(g_pClient->HUD_GetPlayerTeam);

	if (!start) // On 4554 == 0x0
		start = (uintptr_t)(g_pClient->HUD_ChatInputPosition);

	auto ptr = FindDownPattern((PCHAR)V("\x0F\xBF\x04\xFF\xFF\xFF\xFF\xFF\xC3"), (PCHAR)V("xxx?????x"), start, (uintptr_t)(g_pClient->ClientFactory));

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = *(PDWORD)(ptr + 4);

	ptr = (ptr - offsetof(extra_player_info_t, team_id));

	return (PVOID)ptr;
}

PVOID COffsets::SCR_UpdateScreen()
{
	auto ptr = FindDownPattern((PCHAR)V("\x55\x8B\xEC\x83\xEC\x10\xA1\xFF\xFF\xFF\xFF\x56\x33\xF6\x3B\xC6\x0F"), (PCHAR)V("xxxxxxx????xxxxxx"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

PVOID COffsets::Host_FilterTime()
{
	auto ptr = FindDownPattern((PCHAR)V("\x55\x8B\xEC\x83\xEC\x08\xD9\x05\xFF\xFF\xFF\xFF\xD8\x1D"), (PCHAR)V("xxxxxxxx????xx"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		ptr = FindDownPattern((PCHAR)V("\x55\x8B\xEC\x83\xE4\xF8\x83\xEC\x08\xD9\x05\xFF\xFF\xFF\xFF\xD8\x1D\xFF\xFF\xFF\xFF\xDF\xE0\xF6\xC4\x41\x75\x3F\xE8\xFF\xFF\xFF\xFF\x85\xC0\x75\x09\xA1"), 
							  (PCHAR)V("xxxxxxxxxxx????xx????xxxxxxxx????xxxxx"), Module[HW].base, Module[HW].end); //4554 NOT TESTED

		if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
		{
			Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
			return NULL;
		}
	}

	return (PVOID)ptr;
}

PVOID COffsets::CL_RecordHUDCommand()
{
	auto ptr = FindDownPattern((PCHAR)V("\x55\x8B\xEC\x83\xEC\x4C\xC6\x45"), (PCHAR)V("xxxxxxxx"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		ptr = FindDownPattern((PCHAR)V("\x83\xEC\x4C\xC6\x44\x24\x00\x03\xE8\xFF\xFF\xFF\xFF\xD8\x25"), (PCHAR)V("xxxxxxxxx????xx"), Module[HW].base, Module[HW].end);

		if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
		{
			Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
			return NULL;
		}
	}

	return (PVOID)ptr;
}

PVOID COffsets::CL_DriftInterpolationAmount()
{
	auto ptr = FindPush((PCHAR)"ex_interp forced down to %i msec\n", Module[HW].base, Module[HW].end);

	ptr = Absolute(FindDownPattern((PCHAR)V("\x56\xE8"), ptr, Module[HW].end, 0x2));

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

PVOID COffsets::PreS_DynamicSound()
{
	auto ptr = Absolute(FindPush((PCHAR)"CL_Parse_Sound: ent = %i, cl.max_edicts %i", Module[HW].base, Module[HW].end) - 0x10);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		ptr = Absolute(FindPush((PCHAR)"CL_Parse_Sound: ent = %i, cl.max_edicts %i", Module[HW].base, Module[HW].end) - 0x11);

		if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
		{
			Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
			return NULL;
		}
	}

	return (PVOID)ptr;
}

PVOID COffsets::EngineMsgs()
{
	auto ptr = FindPush((PCHAR)"-------- Message Load ---------\n", Module[HW].base, Module[HW].end); // .text:01D1CFB7                 push    offset aMessageLoad ; "-------- Message Load ---------\n"
	
	ptr = FindDownPattern((PCHAR)V("\xBF"), ptr, Module[HW].end, 0x1); // hw.dll+1CFC8 - BF A4B39703           - mov edi,hw.dll+13B3A4 { [0397AFC0] }

	auto ptr_enginemsgbase = (svc_func_t*)(*(PDWORD)ptr - sizeof(DWORD));

	if (ptr_enginemsgbase)
	{
		MSG_ReadByte = (HL_MSG_ReadByte)Absolute(((uintptr_t)ptr_enginemsgbase[svc_cdtrack].pfnParse) + 1);
		MSG_ReadShort = (HL_MSG_ReadShort)Absolute(((uintptr_t)ptr_enginemsgbase[svc_stopsound].pfnParse) + 1);
		MSG_ReadLong = (HL_MSG_ReadLong)Absolute(((uintptr_t)ptr_enginemsgbase[svc_version].pfnParse) + 1);
		MSG_ReadFloat = (HL_MSG_ReadFloat)Absolute(((uintptr_t)ptr_enginemsgbase[svc_timescale].pfnParse) + 1);
		MSG_ReadString = (HL_MSG_ReadString)Absolute(((uintptr_t)ptr_enginemsgbase[svc_print].pfnParse) + 1);

		auto CallMSG_ReadCoord = Absolute((uintptr_t)(ptr_enginemsgbase[svc_particle].pfnParse) + 1);

		if (*(PBYTE)(CallMSG_ReadCoord + 0x13) == 0xE8)	// STEAM
			MSG_ReadCoord = (HL_MSG_ReadCoord)Absolute((CallMSG_ReadCoord + 0x14));
		else if (*(PBYTE)(CallMSG_ReadCoord + 0x15) == 0xE8)	// OLD PATCH (SOFTWARE)
			MSG_ReadCoord = (HL_MSG_ReadCoord)Absolute((CallMSG_ReadCoord + 0x16));
		else if (*(PBYTE)(CallMSG_ReadCoord + 0x0E) == 0xE8)	// OLD PATCH
			MSG_ReadCoord = (HL_MSG_ReadCoord)Absolute((CallMSG_ReadCoord + 0x0F));
		else if (*(PBYTE)(CallMSG_ReadCoord + 0x0B) == 0xE8)	// OLD OLD PATCH
			MSG_ReadCoord = (HL_MSG_ReadCoord)Absolute((CallMSG_ReadCoord + 0x0C));
		else
		{
			Utils::TraceLog(V("%s: MSG_ReadCoord not found.\n"), V(__FUNCTION__));
			return NULL;
		}

		MSG_ReadCount = *(PINT*)((INT)(MSG_ReadByte)+1);
		MSG_CurrentSize = *(PINT*)((INT)(MSG_ReadByte)+7);
		MSG_BadRead = *(PINT*)((INT)(MSG_ReadByte)+20);

		auto SVC_SoundBase = (uintptr_t)ptr_enginemsgbase[svc_sound].pfnParse;

		if (*(PBYTE)(SVC_SoundBase + 0x0E) == 0xE8)
		{
			MSG_Buffer = (sizebuf_t*)(*(PDWORD)(SVC_SoundBase + 0x0A));
			MSG_StartBitReading = (HL_MSG_StartBitReading)Absolute(SVC_SoundBase + 0x0F);
			MSG_ReadBits = (HL_MSG_ReadBits)Absolute(SVC_SoundBase + 0x16);
		}
		else if (*(PBYTE)(SVC_SoundBase + 0x0C) == 0xE8)
		{
			MSG_Buffer = (sizebuf_t*)(*(PDWORD)(SVC_SoundBase + 0x08));
			MSG_StartBitReading = (HL_MSG_StartBitReading)Absolute(SVC_SoundBase + 0x0D);
			MSG_ReadBits = (HL_MSG_ReadBits)Absolute(SVC_SoundBase + 0x14);
		}
		else
		{
			Utils::TraceLog(V("%s: MSG_StartBitReading not found.\n"), V(__FUNCTION__));
			return NULL;
		}

		if (*(PBYTE)(SVC_SoundBase + 0xD6) == 0xE8)
		{
			MSG_EndBitReading = (HL_MSG_EndBitReading)Absolute(SVC_SoundBase + 0xD7);
			MSG_ReadBitVec3Coord = (HL_MSG_ReadBitVec3Coord)Absolute(SVC_SoundBase + 0xAF);
		}
		else if (*(PBYTE)(SVC_SoundBase + 0xE2) == 0xE8)
		{
			MSG_EndBitReading = (HL_MSG_EndBitReading)Absolute(SVC_SoundBase + 0xE3);
			MSG_ReadBitVec3Coord = (HL_MSG_ReadBitVec3Coord)Absolute(SVC_SoundBase + 0xBE);
		}
		else
		{
			Utils::TraceLog(V("%s: MSG_EndBitReading not found.\n"), V(__FUNCTION__));
			return NULL;
		}
	}
	else
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr_enginemsgbase;
}

PVOID COffsets::ClientUserMsgs()
{
	auto ptr = FindPush((PCHAR)"UserMsg: Not Present on Client %d\n", Module[HW].base, Module[HW].end);

	ptr = FindUpPattern((PCHAR)V("\x8B\x35"), ptr, Module[HW].base, 0x2);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = **(PDWORD*)ptr;

	return (PVOID)ptr;
}

PVOID COffsets::ClientStatic()
{
	auto ptr = FindPush((PCHAR)"in :  %i %.2f k/s", Module[HW].base, Module[HW].end);

	ptr = FindUpPattern((PCHAR)V("\xA1\xFF\xFF\xFF\xFF\x83\xEC\x08"), (PCHAR)V("x????xxx"), ptr, Module[HW].base);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = *(uintptr_t*)(ptr + 1);

	ptr = (uintptr_t)(client_static_t*)(ptr - offsetof(client_static_t, netchan.incoming_sequence));

	/*auto ptr = *(PDWORD)(FindPush((PCHAR)"Received signon %i when at %i\n", Module[HW].base, Module[HW].end, 0x12 + 0x1));  // hw.dll+1E383 - A3 BCCB8904           - mov [hw.dll+105CBBC],eax { [00000000] }

	ptr = (uintptr_t)(client_static_t*)(ptr - offsetof(client_static_t, signon));

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}*/

	return (PVOID)ptr;
}

PVOID COffsets::ClientState()
{
	auto ptr = FindDownPattern((PCHAR)V("\x8D\x34\x95\xFF\xFF\xFF\xFF\x56\xC7\x46\x38\x00\x00\x80\xBF\x89\x7E\x40\x89\x7E\x44\x89\x7E\x48"), (PCHAR)V("xxx????xxxxxxxxxxxxxxxxx"), Module[HW].base, Module[HW].end);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	ptr = *(DWORD*)(ptr + 3);

	ptr = (uintptr_t)(client_state_t*)(ptr - offsetof(client_state_t, commands));

	/*auto ptr = *(PDWORD)((uintptr_t)g_pStudio->PlayerInfo + 0xA + 0x2); //hw.dll+87B8A - 3B 05 8433A404        - cmp eax,[hw.dll+1203384] { [00000000] }

	ptr = (uintptr_t)(client_state_t*)(ptr - offsetof(client_state_t, maxclients));

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}*/

	return (PVOID)ptr;
}

PVOID COffsets::PlayerMove()
{
	auto ptr = *(PDWORD)FindPush((PCHAR)"ScreenFade", Module[HW].base, Module[HW].end, 0x18);

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

PVOID COffsets::Studio()
{
	auto ptr = *(PDWORD)((uintptr_t)g_pClient->HUD_GetStudioModelInterface + 0x30);

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		ptr = *(PDWORD)((uintptr_t)g_pClient->HUD_GetStudioModelInterface + 0x1A);

		if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
		{
			Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
			return NULL;
		}
	}

	return (PVOID)ptr;
}

PVOID COffsets::Engine()
{
	auto ptr = *(PDWORD)(FindPush((PCHAR)"sprites/voiceicon.spr", Module[CLIENT].base, Module[CLIENT].end, 0x07));

	if (AddressNotInSpace(ptr, Module[CLIENT].base, Module[CLIENT].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)(ptr);
}

PVOID COffsets::Client()
{
	auto ptr = *(PDWORD)(FindPush((PCHAR)"ScreenFade", Module[HW].base, Module[HW].end, 0x13));

	if (AddressNotInSpace(ptr, Module[HW].base, Module[HW].end))
	{
		Utils::TraceLog(V("%s: not found.\n"), V(__FUNCTION__));
		return NULL;
	}

	return (PVOID)ptr;
}

bool COffsets::AddressNotInSpace(uintptr_t ptr, uintptr_t low, uintptr_t high)
{
	if (low > high)
	{
		const auto reverse = high;
		high = low;
		low = reverse;
	}

	return (ptr < low) || (ptr > high);
}

uintptr_t COffsets::FindPush(PCHAR message, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	char bPushAddrPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	auto ptr = FindDownPattern(message, start, end);
	*(PDWORD)& bPushAddrPattern[1] = ptr;
	ptr = FindDownPattern((PCHAR)bPushAddrPattern, start, end);
	return ptr + offset;
}

bool COffsets::FindModuleByName(const char* name, ModuleInfo* module)
{
	if (!name || !strlen(name))
		return false;

	HMODULE hModuleDll = GetModuleHandle(name);

	if (!hModuleDll)
		return false;

	if (hModuleDll == INVALID_HANDLE_VALUE)
		return false;

	MEMORY_BASIC_INFORMATION mem;

	if (!VirtualQuery(hModuleDll, &mem, sizeof(mem)))
		return false;

	if (mem.State != MEM_COMMIT)
		return false;

	if (!mem.AllocationBase)
		return false;

	IMAGE_DOS_HEADER* dos = (IMAGE_DOS_HEADER*)mem.AllocationBase;
	IMAGE_NT_HEADERS* pe = (IMAGE_NT_HEADERS*)((uintptr_t)dos + (uintptr_t)dos->e_lfanew);

	if (pe->Signature != IMAGE_NT_SIGNATURE)
		return false;

	module->handle = hModuleDll;
	module->base = (uintptr_t)(mem.AllocationBase);
	module->size = (uintptr_t)(pe->OptionalHeader.SizeOfImage);
	module->end = module->base + module->size - 1;

	return true;
}

uintptr_t COffsets::FindUpPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (start && end)
	{
		const auto patternLength = strlen(pattern);

		for (auto i = start; i > end - patternLength; --i)
		{
			bool found = true;

			for (size_t idx = 0; idx < patternLength; ++idx)
			{
				if (pattern[idx] != *(PCHAR)(i + idx))
				{
					found = false;
					break;
				}
			}

			if (found)
				return i + offset;
		}
	}

	return 0;
}

uintptr_t COffsets::FindUpPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (start && end)
	{
		const auto patternLength = strlen(pattern);

		for (auto i = start; i > end - patternLength; --i)
		{
			bool found = true;

			for (size_t idx = 0; idx < patternLength; ++idx)
			{
				if (mask[idx] == 'x' && pattern[idx] != *(PCHAR)(i + idx))
				{
					found = false;
					break;
				}
			}

			if (found)
				return i + offset;
		}
	}

	return 0;
}

uintptr_t COffsets::FindDownPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (start && end)
	{
		const auto patternLength = strlen(pattern);

		for (auto i = start; i < end - patternLength; ++i)
		{
			bool found = true;

			for (size_t idx = 0; idx < patternLength; ++idx)
			{
				if (mask[idx] == 'x' && pattern[idx] != *(PCHAR)(i + idx))
				{
					found = false;
					break;
				}
			}

			if (found)
				return i + offset;
		}
	}

	return 0;
}

uintptr_t COffsets::FindDownPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (start && end)
	{
		const auto patternLength = strlen(pattern);

		for (auto i = start; i < end - patternLength; ++i)
		{
			bool found = true;

			for (size_t idx = 0; idx < patternLength; ++idx)
			{
				if (pattern[idx] != *(PCHAR)(i + idx))
				{
					found = false;
					break;
				}
			}

			if (found)
				return i + offset;
		}
	}

	return 0;
}

uintptr_t COffsets::Absolute(uintptr_t ptr)
{
	return ptr + *(PDWORD)ptr + 0x4;
}