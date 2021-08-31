#include "framework.h"

static constexpr auto MAX_IMPACT_TIME = 0.5;

void CHitRegister::AllocateMemory()
{
	if (!pBaseInfo.get())
		pBaseInfo = std::make_unique<CBaseHitInfo>();
}

void CHitRegister::FreeMemory()
{
	DELETE_UNIQUE_PTR(pBaseInfo);
}

bool CHitRegister::RegisterHit(int index, char* pszSoundFile)
{
	if (g_Local->m_bIsDead)
		return false;

	if (m_iImpactIndex != index)
		return false;

	if (client_state->time - m_flImpactTimeStamp > MAX_IMPACT_TIME)
		return false;

	if (!g_pSound->IsHitSound(pszSoundFile))
		return false;

	if (m_iImpactWeaponID != WEAPON_KNIFE)
		m_iMissedShots--;

	pBaseInfo->m_iHits++;
	pBaseInfo->m_iIndex = m_iImpactIndex;
	pBaseInfo->m_bHeadshot = strstr(pszSoundFile, "headshot") || strstr(pszSoundFile, "bhit_helmet");
	pBaseInfo->m_iWeaponID = m_iImpactWeaponID;
	pBaseInfo->m_iHitboxNum = pBaseInfo->m_bHeadshot ? HITBOX_HEAD : m_iImpactHitboxNum;
	pBaseInfo->m_iHitgroup = Game::GetHitgroup(pBaseInfo->m_iHitboxNum);

	if (g_Player[index]->m_iArmorType == ARMOR_NONE)
	{
		if (strstr(pszSoundFile, "bhit_helmet"))
			g_Player[index]->m_iArmorType = ARMOR_VESTHELM;

		if (strstr(pszSoundFile, "bhit_kevlar"))
			g_Player[index]->m_iArmorType = ARMOR_KEVLAR;
	}

	if (g_Player[index]->m_iArmorType == ARMOR_KEVLAR)
	{
		if (strstr(pszSoundFile, "bhit_helmet"))
			g_Player[index]->m_iArmorType = ARMOR_VESTHELM;

		if (strstr(pszSoundFile, "bhit_flesh") && pBaseInfo->m_iHitgroup == HITGROUP_CHEST)
			g_Player[index]->m_iArmorType = ARMOR_NONE;
	}

	if (g_Player[index]->m_iArmorType == ARMOR_VESTHELM)
	{
		if (strstr(pszSoundFile, "bhit_flesh") && pBaseInfo->m_iHitgroup == HITGROUP_CHEST)
			g_Player[index]->m_iArmorType = ARMOR_NONE;

		if (strstr(pszSoundFile, "headshot"))
			g_Player[index]->m_iArmorType = ARMOR_NONE;
	}

	if (m_iImpactWeaponID == WEAPON_KNIFE)
	{
		if (m_iImpactAttackType == 1)
		{
			if (strstr(pszSoundFile, "bhit_helmet"))
				pBaseInfo->m_iDamage = 40;

			if (strstr(pszSoundFile, "headshot"))
				pBaseInfo->m_iDamage = 60;

			if (strstr(pszSoundFile, "bhit_kevlar"))
				pBaseInfo->m_iDamage = 12;

			if (strstr(pszSoundFile, "bhit_flesh"))
				pBaseInfo->m_iDamage = 15;
		}

		if (m_iImpactAttackType == 2)
		{
			if (strstr(pszSoundFile, "bhit_helmet"))
				pBaseInfo->m_iDamage = 60;

			if (strstr(pszSoundFile, "headshot"))
				pBaseInfo->m_iDamage = 260;

			if (strstr(pszSoundFile, "bhit_kevlar"))
				pBaseInfo->m_iDamage = 65;

			if (strstr(pszSoundFile, "bhit_flesh"))
				pBaseInfo->m_iDamage = 65;
		}

		pBaseInfo->m_iDamage = static_cast<int>(Game::TraceAttack(static_cast<float>(pBaseInfo->m_iDamage), pBaseInfo->m_iHitgroup));
	}
	else
	{
		if (m_iImpactWeaponID)
		{
			pBaseInfo->m_iDamage = Game::TakeSimulatedDamage(g_Local->m_vecEyePos, g_Player[index]->m_vecOrigin, pBaseInfo->m_iHitgroup, index);

			if (pBaseInfo->m_iDamage <= g_Player[index]->m_iHealth)
				g_Player[index]->m_iHealth -= pBaseInfo->m_iDamage;

			//g_pNotifications->Push("Hit registration", Utils::FormatString("pBaseInfo->m_iDamage: %i", pBaseInfo->m_iDamage));

			/*if (pBaseInfo->m_iHitgroup == HITGROUP_STOMACH || pBaseInfo->m_iHitgroup == HITGROUP_CHEST)
			{
				Vector vecForward, vecPlayerForward;

				g_Local->m_QAngles.AngleVectors(&vecForward, NULL, NULL);
				g_Player[index]->m_QAngles.AngleVectors(&vecPlayerForward, NULL, NULL);

				vecForward.z = 0;
				vecPlayerForward.z = 0;

				vecForward.Normalize();
				vecPlayerForward.Normalize();

				if (vecPlayerForward.Dot2D(vecForward) > -0.8f)
				{
					float flRatio = 1.f;

					if (pBaseInfo->m_iHitgroup == HITGROUP_STOMACH)
					{
						flRatio = 0.625f;
					}
					else
					{
						if (pBaseInfo->m_iHitboxNum == HITBOX_LOWER_CHEST ||
							pBaseInfo->m_iHitboxNum == HITBOX_CHEST)
						{
							flRatio = 0.8f;
						}
					}

					pBaseInfo->m_iDamage = static_cast<int>(pBaseInfo->m_iDamage / flRatio);
				}
			}*/
		}
	}

	pBaseInfo->m_flTimeStamp = static_cast<float>(client_state->time);

	ClearImpactInfo();

	return true;
}

void CHitRegister::Update(usercmd_s* cmd)
{
	static int prevweaponclip, prevweaponid;

	if (g_Weapon->m_iWeaponID == prevweaponid && g_Weapon->m_iClip < prevweaponclip)
		m_iMissedShots++;

	prevweaponclip = g_Weapon->m_iClip;
	prevweaponid = g_Weapon->m_iWeaponID;

	if (m_flImpactTimeStamp && client_state->time - m_flImpactTimeStamp > MAX_IMPACT_TIME)
		ClearImpactInfo();

	UpdateAttackInfo(cmd);

	if (g_Weapon.IsKnife())
	{
		float flDistance = 0.f;

		if (m_bIsInAttack)
			flDistance = 80.f;

		if (m_bIsInAttack2)
			flDistance = 64.f;

		if (!flDistance)
			return;

		QAngle QViewAngles = cmd->viewangles;

		QViewAngles.Normalize();

		Vector vecSpreadDir;

		QViewAngles.AngleVectors(&vecSpreadDir, NULL, NULL);

		vecSpreadDir.Normalize();

		Vector vecSrc = g_Local->m_vecEyePos;

		for (int i = 1; i <= client_state->maxclients; i++)
		{
			cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

			if (!Game::IsValidEntity(pGameEntity))
				continue;

			if (g_Player[i]->m_bIsLocal)
				continue;

			if (!g_Player[i]->m_bIsInPVS)
				continue;

			if (g_Player[i]->m_bIsDead)
				continue;

			Vector backtrack;

			Physent::SetSolid(i, SOLID_BBOX);

			if (Game::GetBacktrackOrigin(pGameEntity, backtrack))
				Physent::SetOrigin(i, backtrack);

			pmtrace_t tr;

			g_Engine.pEventAPI->EV_SetTraceHull(HULL_POINT);
			g_Engine.pEventAPI->EV_PlayerTrace(vecSrc, vecSrc + vecSpreadDir * flDistance, PM_NORMAL, -1, &tr);

			if (g_Engine.pEventAPI->EV_IndexFromTrace(&tr) == i)
			{
				m_iImpactIndex = i;
				m_iImpactWeaponID = g_Weapon->m_iWeaponID;
				m_iImpactAttackType = m_bIsInAttack2 ? 2 : 1;
				m_flImpactTimeStamp = static_cast<float>(client_state->time);
			}
		}
	}

	if (g_Weapon.IsGun() && m_bIsInAttack)
	{
		QAngle QViewAngles = cmd->viewangles;

		QViewAngles.x += client_state->punchangle.x * 2.f;
		QViewAngles.y += client_state->punchangle.y * 2.f;

		QViewAngles.Normalize();

		Vector vecForward, vecRight, vecUp, vecRandom;

		QViewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		g_pNoSpread->GetSpreadXY(g_Weapon->m_iRandomSeed, 1, vecRandom);

		Vector vecSpreadDir = vecForward + vecRight * vecRandom.x + vecUp * vecRandom.y;

		vecSpreadDir.Normalize();

		for (int i = 1; i <= client_state->maxclients; i++)
		{
			cl_entity_s* pGameEntity = g_Engine.GetEntityByIndex(i);

			if (!Game::IsValidEntity(pGameEntity))
				continue;

			if (g_Player[i]->m_bIsLocal)
				continue;

			if (!g_Player[i]->m_bIsInPVS)
				continue;

			if (g_Player[i]->m_bIsDead)
				continue;

			for (int j = 0; j < HITBOX_MAX - 1; j++)
			{
				matrix3x4_t hitbox_matrix = g_Player[i]->m_matHitbox[j];

				Vector backtrack;

				if (Game::GetBacktrackOrigin(pGameEntity, backtrack))
				{
					Vector matrix_origin = backtrack + hitbox_matrix.GetOrigin() - g_Player[i]->m_vecOrigin;
					hitbox_matrix.SetOrigin(matrix_origin);
				}

				auto ray_fraction = -1.f; 
				auto ray_hitside = 0;
				auto ray_startsolid = false;

				if (Math::IntersectRayWithOBB(g_Local->m_vecEyePos, vecSpreadDir * g_Weapon->m_flDistance, hitbox_matrix,
					g_Player[i]->m_vecOBBMin[j], g_Player[i]->m_vecOBBMax[j], ray_fraction, ray_hitside, ray_startsolid))
				{	
					bool bSkipHitbox = false;

					for (int k = 0; k < HITBOX_MAX; k++)
					{
						if (k == j)
							continue;

						if (k == HITBOX_SHIELD && !g_Player[i]->m_bHasShield)
							continue;

						hitbox_matrix = g_Player[i]->m_matHitbox[k];

						if (Game::GetBacktrackOrigin(pGameEntity, backtrack))
						{
							Vector matrix_origin = backtrack + hitbox_matrix.GetOrigin() - g_Player[i]->m_vecOrigin;
							hitbox_matrix.SetOrigin(matrix_origin);
						}

						auto ray_fraction2 = -1.f;
						auto ray_hitside2 = 0;
						auto ray_startsolid2 = false;

						if (Math::IntersectRayWithOBB(g_Local->m_vecEyePos, vecSpreadDir * g_Weapon->m_flDistance, hitbox_matrix, 
							g_Player[i]->m_vecOBBMin[k], g_Player[i]->m_vecOBBMax[k], ray_fraction2, ray_hitside2, ray_startsolid2))
						{
							if (ray_fraction2 <= ray_fraction)
							{
								bSkipHitbox = true;
								break;
							}
						}
					}

					if (bSkipHitbox)
						continue;

					m_iImpactIndex = i;
					m_iImpactHitboxNum = j;
					m_iImpactWeaponID = g_Weapon->m_iWeaponID;
					m_flImpactTimeStamp = static_cast<float>(client_state->time);
				}
			}
		}
	}

	if (client_state->time - m_flImpactTimeStamp > MAX_IMPACT_TIME)
	{
		pBaseInfo->m_iMisses += m_iMissedShots;
		m_iMissedShots = 0;
	}

	//g_Engine.Con_NPrintf(10, "pBaseInfo->m_iHits: %i", pBaseInfo->m_iHits);
	//g_Engine.Con_NPrintf(11, "pBaseInfo->m_iMisses: %i", pBaseInfo->m_iMisses);
}

void CHitRegister::Clear()
{
	ClearAttackInfo();
	ClearImpactInfo();
}

CBaseHitInfo* CHitRegister::operator->()
{
	return pBaseInfo.get();
}

void CHitRegister::UpdateAttackInfo(usercmd_s* cmd)
{
	m_bIsInAttack = (cmd->buttons & IN_ATTACK) && !m_bIsInAttack2;
	m_bIsInAttack2 = (cmd->buttons & IN_ATTACK2) && !m_bIsInAttack;
}

void CHitRegister::ClearAttackInfo()
{
	m_bIsInAttack = false;
	m_bIsInAttack2 = false;
}

void CHitRegister::ClearImpactInfo()
{
	m_iImpactIndex = -1;
	m_iImpactWeaponID = -1;
	m_iImpactHitboxNum = -1;
	m_iImpactAttackType = 0;
	m_flImpactTimeStamp = 0.f;
}

CHitRegister g_HitRegister;