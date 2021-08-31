#include "main.h"

svc_func_t *g_pClientEngineMsgs = nullptr;

std::map<svc_commands_e, pfnEngineMessage> g_ClientEngineMsgsMap;

int MSG_SavedReadCount = 0;
int* MSG_ReadCount = nullptr;
int* MSG_CurrentSize = nullptr;
int* MSG_BadRead = nullptr;
sizebuf_t* MSG_Buffer = nullptr;
HL_MSG_ReadByte MSG_ReadByte = nullptr;
HL_MSG_ReadShort MSG_ReadShort = nullptr;
HL_MSG_ReadLong MSG_ReadLong = nullptr;
HL_MSG_ReadFloat MSG_ReadFloat = nullptr;
HL_MSG_ReadString MSG_ReadString = nullptr;
HL_MSG_ReadCoord MSG_ReadCoord = nullptr;
HL_MSG_ReadBitVec3Coord MSG_ReadBitVec3Coord = nullptr;
HL_MSG_ReadBits MSG_ReadBits = nullptr;
HL_MSG_StartBitReading MSG_StartBitReading = nullptr;
HL_MSG_EndBitReading MSG_EndBitReading = nullptr;

static void MSG_SaveReadCount()
{
	MSG_SavedReadCount = *MSG_ReadCount;
}

static void MSG_RestoreReadCount()
{
	*MSG_ReadCount = MSG_SavedReadCount;
}

static void SVC_SendCvarValue()
{
	MSG_SaveReadCount();

	const char* pszCvar = MSG_ReadString();

	if (pszCvar)
	{
		auto *pCvar = g_Engine.pfnGetCvarPointer(pszCvar);

		if (pCvar != nullptr)
		{
			if (strstr(pCvar->name, "cl_minmodels") && pCvar->value == 1)
			{
				pCvar->value = 0;

				MSG_RestoreReadCount();

				g_ClientEngineMsgsMap[svc_sendcvarvalue2]();

				pCvar->value = 1;
				return;
			}
		}
	}

	MSG_RestoreReadCount();

	g_ClientEngineMsgsMap[svc_sendcvarvalue]();
}

static void SVC_SendCvarValue2()
{
	MSG_SaveReadCount();

	MSG_ReadLong();

	const char* pszCvar = MSG_ReadString();

	if (pszCvar)
	{
		auto *pCvar = g_Engine.pfnGetCvarPointer(pszCvar);

		if (pCvar != nullptr)
		{
			if (strstr(pCvar->name, "cl_minmodels") && pCvar->value == 1)
			{
				pCvar->value = 0;

				MSG_RestoreReadCount();

				g_ClientEngineMsgsMap[svc_sendcvarvalue2]();

				pCvar->value = 1;
				return;
			}
		}
	}

	MSG_RestoreReadCount();

	g_ClientEngineMsgsMap[svc_sendcvarvalue2]();
}

static void SVC_DeltaPacketEntities()
{
	g_ClientEngineMsgsMap[svc_deltapacketentities]();

	if (client_state && cvar.replace_zombie_models)
	{
		auto frameid = client_state->validsequence % IM_ARRAYSIZE(client_state->frames);
		auto curframe = client_state->frames[frameid];
		auto packet_entities = curframe.packet_entities;

		if (packet_entities.num_entities && packet_entities.entities && client_state->model_precache_count)
		{
			int modelindex_t = 0;
			int modelindex_ct = 0;
			int modelindex_vip = 0;

			for (int x = 0; x < client_state->model_precache_count; x++)
			{
				if (!client_state->model_precache[x])
					continue;
				
				if (strstr(client_state->model_precache[x]->name, "leet.mdl"))
					modelindex_t = x;
				else if (strstr(client_state->model_precache[x]->name, "gign.mdl"))
					modelindex_ct = x;
				else if (strstr(client_state->model_precache[x]->name, "vip.mdl"))
					modelindex_vip = x;
			}

			if (modelindex_t && modelindex_ct && modelindex_vip)
			{
				for (int i = 0; i < packet_entities.num_entities; i++)
				{
					if (packet_entities.entities[i].aiment >= 1 && packet_entities.entities[i].aiment <= MAX_CLIENTS && packet_entities.entities[i].aiment != g_Local.m_iIndex)
					{
						const auto pPlayer = g_World.GetPlayer(i);

						if(pPlayer->m_iTeam == CT)
							packet_entities.entities[i].modelindex = modelindex_ct;
						else if (pPlayer->m_iTeam == TERRORIST)
							packet_entities.entities[i].modelindex = modelindex_t;
						else
							packet_entities.entities[i].modelindex = modelindex_vip;
					}
				}
			}
		}
	}
}

bool HookEngineMessages(void)
{
	if (g_pClientEngineMsgs)
	{
		if (!HookEngineMsg(svc_sendcvarvalue, SVC_SendCvarValue))
			return false;

		if (!HookEngineMsg(svc_sendcvarvalue2, SVC_SendCvarValue2))
			return false;

		if (!HookEngineMsg(svc_deltapacketentities, SVC_DeltaPacketEntities))
			return false;
	}

	return true;
}

void UnHookEngineMessages(void)
{
	if (g_pClientEngineMsgs)
	{
		UnHookEngineMsg(svc_sendcvarvalue);
		UnHookEngineMsg(svc_sendcvarvalue2);
		UnHookEngineMsg(svc_deltapacketentities);
	}
}

bool HookEngineMsg(svc_commands_e index, pfnEngineMessage pfn)
{
	auto pClientEngineMsgs = g_pClientEngineMsgs;
	while (pClientEngineMsgs)
	{
		if (pClientEngineMsgs->opcode == index)
		{
			g_ClientEngineMsgsMap[index] = pClientEngineMsgs->pfnParse;
			pClientEngineMsgs->pfnParse = pfn;
			return true;
		}

		pClientEngineMsgs++;
	}

	TraceLog("> %s: failed to hook svc index %i.\n", __FUNCTION__, index);

	return false;
}

bool UnHookEngineMsg(svc_commands_e index)
{
	auto pClientEngineMsgs = g_pClientEngineMsgs;
	while (pClientEngineMsgs)
	{
		if (pClientEngineMsgs->opcode == index && g_ClientEngineMsgsMap[index] != nullptr)
		{
			pClientEngineMsgs->pfnParse = g_ClientEngineMsgsMap[index];
			g_ClientEngineMsgsMap[index] = nullptr;
			return true;
		}

		pClientEngineMsgs++;
	}

	TraceLog("> %s: failed to unhook svc index %i.\n", __FUNCTION__, index);

	return false;
}