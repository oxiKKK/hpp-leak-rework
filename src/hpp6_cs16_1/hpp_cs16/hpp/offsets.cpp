#include "main.h"

COffsets g_Offsets;

bool COffsets::FindModules(void)
{
	if (!FindModuleByName("hw.dll", &hw))
	{
		if (!FindModuleByName("engine.dll", &hw)) //gsclient 7960
		{
			return false;
		}
	}

	if (!FindModuleByName("client.dll", &client))
	{
		if (!FindModuleByName("client_hud.dll", &client)) //gsclient 7960
		{
			return false;
		}
	}

	if (!FindModuleByName("gameui.dll", &gameui))
		return false;

	if (!FindModuleByName("vgui2.dll", &vgui2))
		return false;

	if (!FindModuleByName("gameoverlayrenderer.dll", &gameoverlayrenderer))
		TraceLog("> %s: module gameoverlayrenderer.dll not found.\n", __FUNCTION__);

	if (!FindModuleByName("steamclient.dll", &steamclient))
		TraceLog("> %s: module steamclient.dll not found.\n", __FUNCTION__);

	return true;
}

bool COffsets::FindInterfaces(void)
{
	auto factory_hw = CaptureFactory(&hw);
	auto factory_gameui = CaptureFactory(&gameui);
	auto factory_vgui2 = CaptureFactory(&vgui2);
	auto factory_steamclient = CaptureFactory(&steamclient);

	if (!factory_hw || !factory_gameui || !factory_vgui2)
		return false;

	g_pISurface = (vgui::ISurface*)(CaptureInterface(factory_hw, VGUI_SURFACE_INTERFACE_VERSION));
	g_pIGameUI = (IGameUI*)(CaptureInterface(factory_gameui, GAMEUI_INTERFACE_VERSION));
	g_pIRunGameEngine = (IRunGameEngine*)(CaptureInterface(factory_gameui, RUNGAMEENGINE_INTERFACE_VERSION));
	g_pConsole = (IGameConsole*)(CaptureInterface(factory_gameui, GAMECONSOLE_INTERFACE_VERSION));

	if (!g_pISurface || !g_pIGameUI || !g_pIRunGameEngine || !g_pConsole)
		return false;

	FindSteamInterfaces(factory_steamclient);

	return true;
}

bool COffsets::FindSteamInterfaces(CreateInterfaceFn steamclient)
{
	if (steamclient)
	{
		g_pISteamClient = (ISteamClient*)(CaptureInterface(steamclient, STEAMCLIENT_INTERFACE_VERSION));

		if (g_pISteamClient)
		{
			HSteamPipe hSteamPipe = g_pISteamClient->CreateSteamPipe();

			if (hSteamPipe)
			{
				HSteamUser hSteamUser = g_pISteamClient->ConnectToGlobalUser(hSteamPipe);

				if (hSteamUser)
				{
					g_pISteamScreenshots = g_pISteamClient->GetISteamScreenshots(hSteamUser, hSteamPipe, STEAMSCREENSHOTS_INTERFACE_VERSION);
					g_pISteamUtils = g_pISteamClient->GetISteamUtils(hSteamPipe, STEAMUTILS_INTERFACE_VERSION);
					g_pISteamFriends = g_pISteamClient->GetISteamFriends(hSteamUser, hSteamPipe, STEAMFRIENDS_INTERFACE_VERSION);
					g_pISteamUser = g_pISteamClient->GetISteamUser(hSteamUser, hSteamPipe, STEAMUSER_INTERFACE_VERSION);
					
					if (g_pISteamScreenshots && g_pISteamUtils && g_pISteamFriends && g_pISteamUser)
						return true;
				}
				else {
					g_pISteamClient->BReleaseSteamPipe(hSteamPipe);
					g_pISteamClient->BShutdownIfAllPipesClosed();
					TraceLog("%s: steamuser not found.\n", __FUNCTION__);
				}
			}
			else
				TraceLog("%s: steampipe not found.\n", __FUNCTION__);
		}
		else
			TraceLog("%s: interface steamclient not found.\n", __FUNCTION__);
	}
	else
		TraceLog("%s: factory steamclient not found.\n", __FUNCTION__);

	return false;
}

bool COffsets::FindOffsets(void)
{
	if (!(g_pClient = (cl_clientfunc_t*)(Client())))
		return false;

	if (!(g_pEngine = (cl_enginefunc_t*)(Engine())))
		return false;

	if (!(g_pStudio = (engine_studio_api_t*)(Studio())))
		return false;

	if (!(g_pStudioModelRenderer = (StudioModelRenderer_t*)(StudioRenderModel())))
		return false;

	if (!(pmove = (playermove_t*)(PlayerMove())))
		return false;

	if (!(g_pPreS_DynamicSound = (PreS_DynamicSound_t)(PreS_DynamicSound())))
		return false;

	if (!(g_pClientUserMsgs = (UserMsg)(ClientUserMsgs())))
		return false;

	g_pClientEngineMsgs = (svc_func_t*)(EngineMsgs());

	if (!(client_state = (client_state_t*)(ClientState())))
		return false;

	if (!(client_static = (client_static_t*)(ClientStatic())))
		return false;
	
	/*if (!(g_Globals.m_pbAllowCheats = (bool*)(AllowCheats())))
		return false;*/

	if (!(keybindings = (char**)KeyBindings()))
		return false;

	/*if (!(g_pR_DrawEntitiesOnList = (R_DrawEntitiesOnList_t)R_DrawEntitiesOnList()))
		return false;*/

	if(!(m_dwInitPoint = FindInitPoint()))
		return false;

	if (!(PatchInterpolationTime()))
		return false;

	if (!(m_dwpSpeed = FindSpeed()))
		return false;

	if (!(g_pCL_RecordHUDCommand = (CL_RecordHUDCommand_t)CL_RecordHUDCommand()))
		return false;

	if (!(particles = (particle_t**)Particles()))
		return false;

	if (!g_pEngine->V_CalcShake || !g_pClient->V_CalcRefdef || !g_pStudioModelRenderer->StudioSlerpBones || !g_pStudio->StudioSetupSkin)
		return false;

	if (!(g_pInitiateGameConnection = (InitiateGameConnection_t)InitiateGameConnection()))
		return false;

	if (!(g_Globals.m_iGameBuild = GetGameBuild()))
		return false;

	RtlCopyMemory(&g_Client, g_pClient, sizeof(g_Client));
	RtlCopyMemory(&g_Engine, g_pEngine, sizeof(g_Engine));
	RtlCopyMemory(&g_Studio, g_pStudio, sizeof(g_Studio));
	RtlCopyMemory(&g_StudioModelRenderer, g_pStudioModelRenderer, sizeof(g_StudioModelRenderer));

	if (!GlobalTime())
		return false;

	/*if (g_Globals.m_iGameBuild >= 8212)
	{
		if (!(g_PlayerExtraInfo = (extra_player_info_t*)(ExtraPlayerInfo())))
			return false;
	}
	else
		TraceLog("%s: g_PlayerExtraInfo outdated, please use >= 8212 build.\n", __FUNCTION__);*/

	return true;
}

int COffsets::GetGameBuild()
{
	auto CommandByName = [](const char* name) -> cmd_t*
	{
		auto* cmdlist = g_pEngine->pfnGetCmdList();

		while (cmdlist)
		{
			if (!strcmp(cmdlist->name, name))
				return cmdlist;

			cmdlist = cmdlist->next;
		}

		return nullptr;
	};

	const auto cmd = CommandByName("version");

	if (cmd)
	{
		const auto ptr = Absolute((uintptr_t)cmd->function + 0x17);

		if (ptr)
		{
			using function = int(__cdecl*)();

			const auto GetGameBuildFn = (function)ptr;

			return GetGameBuildFn();
		}
	}

	TraceLog("%s: not found.\n", __FUNCTION__);

	return NULL;
}


uintptr_t COffsets::InitiateGameConnection(void)
{
	auto ptr = FindPush((PCHAR)"%c%c%c%cconnect %i %i \"%s\" \"%s\"\n", hw.base, hw.end);

	ptr = FindUpPattern((PCHAR)"\xE8\xFF\xFF\xFF\xFF\x83\xC4\x1C\x8B\xF8", (PCHAR)"x????xxxxx", ptr, hw.base);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return 0;
	}

	ptr = (intptr_t)(ptr + 5) + *(intptr_t *)(ptr + 1);

	return ptr;
}

bool COffsets::GlobalTime(void)
{
	auto ptr = (uintptr_t)g_Engine.pNetAPI->SendRequest;

	ptr = FindDownPattern((PCHAR)"\x8B\x0D", ptr, hw.end);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return false;
	}

	ptr = *(DWORD*)(ptr + 2);

	g_Globals.m_pGlobalTime = (double*)ptr;

	return true;
}

uintptr_t COffsets::CL_RecordHUDCommand(void)
{
	auto ptr = FindDownPattern((PCHAR)"\x55\x8B\xEC\x83\xEC\x4C\xC6\x45\xB4\x03\xE8\xFF\xFF\xFF\xFF\xD8\x25", (PCHAR)"xxxxxxxxxxx????xx", hw.base, hw.end);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		ptr = FindDownPattern((PCHAR)"\x83\xEC\x4C\xC6\x44\x24\x00\x03\xE8\xFF\xFF\xFF\xFF\xD8\x25", (PCHAR)"xxxxxxxxx????xx", hw.base, hw.end);

		if (AddressNotInSpace(ptr, hw.base, hw.end))
		{
			TraceLog("%s: not found.\n", __FUNCTION__);
			return NULL;
		}
	}

	return ptr;
}

uintptr_t COffsets::FindSpeed(void)
{
	auto ptr = FindDownPattern((PCHAR)"Texture load: %6.1fms", hw.base, hw.end);
	auto ptr_speed = (PVOID)*(DWORD*)(FindReference(hw.base, hw.end, ptr) - 7);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (uintptr_t)ptr_speed;
}

uintptr_t COffsets::Particles(void)
{
	auto ptr = FindPush((PCHAR)"-particles", hw.base, hw.end);

	ptr = FindDownPattern((PCHAR)"\x90", ptr, hw.end, -10);//find end function

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	ptr = *(DWORD*)(ptr + 1);

	return ptr;
}

bool COffsets::PatchInterpolationTime(void)
{
	auto ptr = FindDownPattern((PCHAR)"\x83\xC4\x08\x39\x3D", hw.base, hw.end);

	auto ptr_start_func = FindUpPattern((PCHAR)"\x90\x90", ptr, hw.base);;

	ptr = FindDownPattern((PCHAR)"\xE8\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x56\xE8", (PCHAR)"x????x????xx", ptr_start_func, ptr, 0xB);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return false;
	}

	m_dwInterpolationTime = ptr;

	//backup
	unsigned char* byte_array = (unsigned char*)m_dwInterpolationTime;

	for (auto j = 0; j < sizeof(m_puszPatchInterpolationTimeBackup); j++)
		m_puszPatchInterpolationTimeBackup[j] = (unsigned)byte_array[j];

	unsigned char uszPatch[5] = { 0x90, 0x90, 0x90, 0x90, 0x90 };

	g_Utils.memwrite(m_dwInterpolationTime, (uintptr_t)uszPatch, sizeof(uszPatch));

	return true;
}

uintptr_t COffsets::FindInitPoint(void)
{
	auto ptr = FindDownPattern((PCHAR)"\x8B\x44\x24\x04\x48\x83\xF8\x1D", client.base, client.end);

	if (AddressNotInSpace(ptr, client.base, client.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return ptr;
}

uintptr_t COffsets::R_DrawEntitiesOnList(void)
{
	auto ptr = FindPush((PCHAR)"progs/flame.mdl", hw.base, hw.end);

	ptr = FindDownPattern((PCHAR)"\x90\x90\x90", ptr, hw.end); //Find end function

	while (*(PBYTE)ptr == 0x90) //Find start next function
		ptr++;

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	auto ptr_cl_num_visedicts = FindDownPattern((PCHAR)"\x8B\x0D", ptr, hw.end); //cl_numvisedicts

	if (AddressNotInSpace(ptr_cl_num_visedicts, hw.base, hw.end))
	{
		TraceLog("%s: cl_numvisedicts not found.\n", __FUNCTION__);
		return NULL;
	}

	g_Globals.m_piNumVisibleEntities = (int*)*(DWORD*)(ptr_cl_num_visedicts + 2);

	auto ptr_cl_visedicts = FindDownPattern((PCHAR)"\x8B\x14", ptr_cl_num_visedicts, ptr_cl_num_visedicts + 0x18); //cl_visedicts steam

	if (AddressNotInSpace(ptr_cl_visedicts, hw.base, hw.end))
	{
		ptr_cl_visedicts = FindDownPattern((PCHAR)"\x55\xBE", ptr_cl_num_visedicts, ptr_cl_num_visedicts + 0x18, 1); //cl_visedicts
		g_Globals.m_pVisibleEntities = (cl_entity_t**)*(DWORD*)(ptr_cl_visedicts + 1);

		TraceLog("> %s: find cl_visedicts old version.\n", __FUNCTION__);
	}
	else
		g_Globals.m_pVisibleEntities = (cl_entity_t**)*(DWORD*)(ptr_cl_visedicts + 3);

	return ptr;
}

uintptr_t COffsets::FireBullets(void)
{
	auto ptr = (uintptr_t)(void*)g_Offsets.FindDownPattern((PCHAR)"\xB8\xFF\xFF\xFF\xFF\x53\x8B\x9C\x24\xFF\xFF\xFF\xFF\x89\x44\x24\xFF\x89\x44\x24\xFF\x89\x44\x24\xFF\x8B\x84\x24\xFF\xFF\xFF\xFF\xC7\x44\x24\xFF\xFF\xFF\xFF\xFF\x89\x44\x24\xFF\x8D\x43\xFF\x83\xF8\xFF", (PCHAR)"x????xxxx????xxx?xxx?xxx?xxx????xxx?????xxx?xx?xx?", client.base, client.end, -0x06);

	if (AddressNotInSpace(ptr, client.base, client.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return ptr;
}

uintptr_t COffsets::KeyBindings(void)
{
	auto ptr = FindPush((PCHAR)"unbind <key> : remove commands from a key", hw.base, hw.end);

	ptr = FindUpPattern((PCHAR)"\x8B\x04\xFF\xFF\xFF\xFF\xFF", (PCHAR)"xx?????", ptr, hw.base);

	ptr = *(DWORD*)(ptr + 3);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return ptr;
}

uintptr_t COffsets::EngineMsgs(void)
{
	auto ptr = FindDownPattern((PCHAR)"\xBF\xFF\xFF\xFF\xFF\x8B\x04\xB5\xFF\xFF\xFF\xFF\x85\xC0\x74\xFF\x81\xFF\xFF\xFF\xFF\xFF\x7F\x04\x8B\x0F\xEB\x05", (PCHAR)"x????xxx????xxx?xx????xxxxxx", hw.base, hw.end, 1);

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
			TraceLog("%s: MSG_ReadCoord not found.\n", __FUNCTION__);
			return NULL;
		}

		MSG_ReadCount = *(PINT*)((INT)(MSG_ReadByte)+1);
		MSG_CurrentSize = *(PINT*)((INT)(MSG_ReadByte)+7);
		MSG_BadRead = *(PINT*)((INT)(MSG_ReadByte)+20);

		auto SVC_SoundBase = (uintptr_t)ptr_enginemsgbase[svc_sound].pfnParse;

		if (*(PBYTE)(SVC_SoundBase + 0x0E) == 0xE8)
		{
			MSG_Buffer = (sizebuf_t *)(*(PDWORD)(SVC_SoundBase + 0x0A));
			MSG_StartBitReading = (HL_MSG_StartBitReading)Absolute(SVC_SoundBase + 0x0F);
			MSG_ReadBits = (HL_MSG_ReadBits)Absolute(SVC_SoundBase + 0x16);
		}
		else if (*(PBYTE)(SVC_SoundBase + 0x0C) == 0xE8)
		{
			MSG_Buffer = (sizebuf_t *)(*(PDWORD)(SVC_SoundBase + 0x08));
			MSG_StartBitReading = (HL_MSG_StartBitReading)Absolute(SVC_SoundBase + 0x0D);
			MSG_ReadBits = (HL_MSG_ReadBits)Absolute(SVC_SoundBase + 0x14);
		}
		else
		{
			TraceLog("%s: MSG_StartBitReading not found.\n", __FUNCTION__);
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
			TraceLog("%s: MSG_EndBitReading not found.\n", __FUNCTION__);
			return NULL;
		}
	}
	else
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (uintptr_t)ptr_enginemsgbase;
}

uintptr_t COffsets::ExtraPlayerInfo(void)
{
	auto ptr = FindDownPattern((PCHAR)"\x0F\xBF\x04\xFF\xFF\xFF\xFF\xFF\xC3", (PCHAR)"xxx?????x", (uintptr_t)(g_pClient->HUD_GetPlayerTeam), (uintptr_t)(g_pClient->ClientFactory));

	ptr = *(uintptr_t*)(ptr + 4);

	ptr = (ptr - offsetof(extra_player_info_t, team_id));

	if (AddressNotInSpace(ptr, client.base, client.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return ptr;
}

uintptr_t COffsets::AllowCheats(void)
{
	auto ptr = FindPush((PCHAR)"Server must enable cheats to activate fakelag", hw.base, hw.end, -0x22);

	ptr = *(uintptr_t*)(ptr + 1);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return ptr;
}

PVOID COffsets::ClientStatic(void)
{
	auto ptr = FindPush((PCHAR) "in :  %i %.2f k/s", hw.base, hw.end);

	ptr = FindUpPattern((PCHAR) "\xA1\xFF\xFF\xFF\xFF\x83\xEC\x08", (PCHAR) "x????xxx", ptr, hw.base);

	ptr = *(uintptr_t*)(ptr + 1);

	ptr = (uintptr_t)(client_static_t*)(ptr - offsetof(client_static_t, netchan.incoming_sequence));

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (PVOID)ptr;
}

uintptr_t COffsets::PreS_DynamicSound(void)
{
	auto ptr = Absolute(FindPush((PCHAR) "CL_Parse_Sound: ent = %i, cl.max_edicts %i", hw.base, hw.end) - 0x10);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		ptr = Absolute(FindPush((PCHAR) "CL_Parse_Sound: ent = %i, cl.max_edicts %i", hw.base, hw.end) - 0x11);

		if (AddressNotInSpace(ptr, hw.base, hw.end))
		{
			TraceLog("%s: not found.\n", __FUNCTION__);
			return NULL;
		}
	}

	return ptr;
}

PVOID COffsets::ClientUserMsgs(void)
{
	auto ptr = (uintptr_t)g_pEngine->pfnHookUserMsg;

	ptr = Absolute(FindDownPattern((PCHAR) "\x52\x50\xE8\xFF\xFF\xFF\xFF\x83", (PCHAR) "xxx????x", ptr, ptr + 0x32, 0x3));

	ptr = FindDownPattern((PCHAR) "\xFF\xFF\xFF\x0C\x56\x8B\x35\xFF\xFF\xFF\xFF\x57", (PCHAR) "???xxxx????x", ptr, ptr + 0x32, 0x7);

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (PVOID)UserMsg(**(PDWORD*)ptr);
}

PVOID COffsets::PlayerMove(void)
{
#ifdef LICENSING
	VMProtectBegin(__FUNCTION__);
	auto ptr = FindDownPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	auto ptr_player_move = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, ptr) + m_dwCloudOffsets[1]);

	if (AddressNotInSpace((uintptr_t)ptr_player_move, hw.base, hw.end))
	{
		TraceLog("%s: not found\n.", __FUNCTION__);
		return NULL;
	}
	VMProtectEnd();
#else
	auto ptr = FindDownPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	auto ptr_player_move = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, ptr) + 0x18);

	if (AddressNotInSpace((uintptr_t)ptr_player_move, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}
#endif // LICENSE

	return ptr_player_move;
}

PVOID COffsets::StudioRenderModel(void)
{
	auto ptr = FindDownPattern((PCHAR) "\x56\x8B\xF1\xE8\xFF\xFF\xFF\xFF\xC7\x06\xFF\xFF\xFF\xFF\xC6\x86\xFF\xFF\xFF\xFF\xFF\x8B\xC6\x5E\xC3", (PCHAR) "xxxx????xx????xx?????xxxx", client.base, client.end);

	if (AddressNotInSpace(ptr, client.base, client.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (PVOID) * (DWORD*)(ptr + 0x0A);
}

PVOID COffsets::Studio(void)
{
	auto ptr_studio = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x30);

	if (AddressNotInSpace((uintptr_t)(ptr_studio), client.base, client.end))
	{
		ptr_studio = (engine_studio_api_t*)*(DWORD*)((DWORD)g_pClient->HUD_GetStudioModelInterface + 0x1A);

		if (AddressNotInSpace((uintptr_t)(ptr_studio), client.base, client.end))
		{
			TraceLog("%s: not found.\n", __FUNCTION__);
			return NULL;
		}
	}

	return ptr_studio;
}

PVOID COffsets::Engine(void)
{
	auto ptr = *(PDWORD)(FindPush((PCHAR) "sprites/voiceicon.spr", client.base, client.end) + 0x07);

	if (AddressNotInSpace(ptr, client.base, client.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (PVOID)(ptr);
}

PVOID COffsets::Client(void)
{
#ifdef LICENSING
	VMProtectBegin(__FUNCTION__);
	auto ptr = FindDownPattern((PCHAR)"ScreenFade", hw.base, hw.end);
	auto ptr_client = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, ptr) + m_dwCloudOffsets[0]); // 0x13 for cloud

	if (AddressNotInSpace((uintptr_t)(ptr_client), hw.base, hw.end))
	{
		//TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}
	VMProtectEnd();
#else
	auto ptr = FindDownPattern((PCHAR) "ScreenFade", hw.base, hw.end);
	auto ptr_client = (PVOID) * (PDWORD)(FindReference(hw.base, hw.end, ptr) + 0x13); // 0x13 for cloud

	if (AddressNotInSpace((uintptr_t)(ptr_client), hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}
#endif // LICENSE

	return ptr_client;
}

PVOID COffsets::ClientState(void)
{
	auto ptr = FindDownPattern((PCHAR)"\x8D\x34\x95\xFF\xFF\xFF\xFF\x56\xC7\x46\x38\x00\x00\x80\xBF\x89\x7E\x40\x89\x7E\x44\x89\x7E\x48", (PCHAR)"xxx????xxxxxxxxxxxxxxxxx", hw.base, hw.end);

	ptr = *(DWORD*)(ptr + 3);

	ptr = (uintptr_t)(client_state_t*)(ptr - offsetof(client_state_t, commands));

	if (AddressNotInSpace(ptr, hw.base, hw.end))
	{
		TraceLog("%s: not found.\n", __FUNCTION__);
		return NULL;
	}

	return (PVOID)ptr;
}

uintptr_t COffsets::FindSteamOverlay(void)
{
	if (!gameoverlayrenderer.base || !gameoverlayrenderer.end)
		return NULL;

	auto ptr = FindDownPattern((PCHAR)"\xFF\x15\xFF\xFF\xFF\xFF\x80\x3D\xFF\xFF\xFF\xFF\xFF\x8B\xF0\x74\x0C\x68\xFF\xFF\xFF\xFF\xE8\xFF\xFF\xFF\xFF\x8B\xC6\x5E\x8B\xE5\x5D\xC2\x04\x00", (PCHAR)"xx????xx?????xxxxx????x????xxxxxxxxx", gameoverlayrenderer.base, gameoverlayrenderer.end, 0x02);

	if (!ptr)
		return NULL;

	ptr = *(uintptr_t*)ptr;

	if (!ptr)
		return NULL;

	ptr = *(uintptr_t*)ptr;

	return ptr;
}

uintptr_t COffsets::Absolute(uintptr_t ptr)
{
	return ptr + *(PDWORD)ptr + 0x4;
}

uintptr_t COffsets::FindReference(uintptr_t start, uintptr_t end, uintptr_t ptr)
{
	char szPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	*(PDWORD)&szPattern[1] = ptr;
	return FindDownPattern(szPattern, start, end, 0);
}

uintptr_t COffsets::FindPush(PCHAR message, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	char bPushAddrPattern[] = { 0x68, 0x00, 0x00, 0x00, 0x00, 0x00 };
	auto ptr = FindDownPattern(message, start, end);
	*(PDWORD)&bPushAddrPattern[1] = ptr;
	ptr = FindDownPattern((PCHAR)bPushAddrPattern, start, end);
	return ptr + offset;
}

bool COffsets::AddressNotInSpace(uintptr_t ptr, uintptr_t low, uintptr_t high)
{
	if (low > high)
	{
		auto reverse = high;
		high = low;
		low = reverse;
	}

	return ((ptr < low) || (ptr > high));
}

void COffsets::Error(const char *fmt, ...)
{
	va_list argptr;
	static char string[4096];

	va_start(argptr, fmt);
	vsnprintf_s(string, sizeof(string), fmt, argptr);
	va_end(argptr);

	MessageBox(0, string, "Hpp Hack", MB_OK | MB_ICONERROR);
	TerminateProcess(GetCurrentProcess(), 0);
}

bool COffsets::FindModuleByName(const char* name, Module* module)
{
	if (!name || !*name || !module)
		return false;

	HMODULE hModuleDll = GetModuleHandle(name);

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

	module->handle = hModuleDll;
	module->base = (uintptr_t)(mem.AllocationBase);
	module->size = (uintptr_t)(pe->OptionalHeader.SizeOfImage);
	module->end = module->base + module->size - 1;

	return true;
}

uintptr_t COffsets::FindUpPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (!start || !end)
		return 0;

	size_t patternLength = strlen(pattern);

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

	return 0;
}

uintptr_t COffsets::FindUpPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (!start || !end)
		return 0;

	size_t patternLength = strlen(pattern);

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

	return 0;
}

uintptr_t COffsets::FindDownPattern(PCHAR pattern, PCHAR mask, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (!start || !end)
		return 0;

	size_t patternLength = strlen(pattern);

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

	return 0;
}

uintptr_t COffsets::FindDownPattern(PCHAR pattern, uintptr_t start, uintptr_t end, uintptr_t offset)
{
	if (!start || !end)
		return 0;

	size_t patternLength = strlen(pattern);

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

	return 0;
}