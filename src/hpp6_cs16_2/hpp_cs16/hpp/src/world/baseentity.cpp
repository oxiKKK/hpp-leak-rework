#include "framework.h"

void CBaseEntity::AllocateMemory()
{
	for (auto& pEntity : pBaseInfo)
	{
		if (pEntity.get())
			continue;

		pEntity = std::make_unique<CBaseEntInfo>();
		pEntity->m_ClassId = EClassEntity_BaseEntity;
	}
}

void CBaseEntity::FreeMemory()
{
	for (auto& pEntity : pBaseInfo)
		DELETE_UNIQUE_PTR(pEntity);
}

void CBaseEntity::Update()
{
	for (int i = 1; i <= client_state->max_edicts; i++)
	{
		if (i <= client_state->maxclients)
		{
			*pBaseInfo[i - 1].get() = *g_Player.GetBaseInfo(i);
			continue;
		}

		cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

		if (!pGameEntity)
			continue;

		if (!pGameEntity->model)
			continue;

		if (!pGameEntity->index)
			continue;

		UpdateEntity(pGameEntity);
	}
}

CBaseEntInfo* CBaseEntity::operator[](int index)
{
	assert(index > 0 && index <= MAX_EDICTS);

	return pBaseInfo[index - 1].get();
}

void CBaseEntity::UpdateEntity(cl_entity_s* pGameEntity)
{
	auto* pEntity = pBaseInfo[pGameEntity->index - 1].get();

	pEntity->m_bIsConnected = true;
	pEntity->m_bIsInPVS = false;
	pEntity->m_iEntIndex = pGameEntity->index;
	pEntity->m_iMessageNum = pGameEntity->curstate.messagenum;

	strcpy_s(pEntity->m_szModelName, pGameEntity->model->name);

	if (pEntity->m_iMessageNum < g_Local->m_iMessageNum)
		return;

	pEntity->m_bIsInPVS = true;
	pEntity->m_flLastTimeInPVS = static_cast<float>(client_state->time);
	pEntity->m_vecOrigin = pGameEntity->origin;
	pEntity->m_flDistance = g_Local->m_vecOrigin.Distance(pGameEntity->origin);

	if (strstr(pEntity->m_szModelName, "/p_"))
	{
		for (int i = 1; i <= client_state->maxclients; i++)
		{
			if (g_Player[i]->m_vecOrigin.Distance(pEntity->m_vecOrigin) <= sqrt(2.f) * 16.f)
				strcpy_s(g_Player[i]->m_szWeaponModelName, pEntity->m_szModelName);
		}
	}
	else
	{
		if (strstr(pEntity->m_szModelName, "player.mdl") && pGameEntity->curstate.effects & EF_NODRAW)
		{
			Physent::SetMaxs(pEntity->m_iEntIndex, Vector());
			Physent::SetMins(pEntity->m_iEntIndex, Vector());
			Physent::SetSolid(pEntity->m_iEntIndex, SOLID_NOT);
		}
	}
}

CBaseEntity g_Entity;