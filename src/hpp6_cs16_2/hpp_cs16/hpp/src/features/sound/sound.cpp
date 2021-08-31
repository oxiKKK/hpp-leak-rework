#include "framework.h"

CSound::~CSound() 
{
	ClearResolverData();
}

bool CSound::IsValidSound(char* pszSoundFile, float* pOrigin, int index, DWORD channel)
{
	if (!pszSoundFile)
		return false;

	if (!strlen(pszSoundFile))
		return false;

	if (!pOrigin)
		return false;

	if (Vector(pOrigin).IsZero())
		return false;

	if (index <= 0 || index > MAX_EDICTS)
		return false;

	/*if (g_Entity[index]->m_bIsLocal)
		return false;*/

	if (channel == CHAN_AUTO)
		return false;

	return true;
}

bool CSound::IsPlayerSound(char* pszSoundFile)
{
	return strstr(pszSoundFile, "player");
}

bool CSound::IsWeaponSound(char* pszSoundFile)
{
	return strstr(pszSoundFile, "weapons");
}

bool CSound::IsHitSound(char* pszSoundFile)
{
	if (strstr(pszSoundFile, "bhit_helmet"))
		return true;

	if (strstr(pszSoundFile, "headshot"))
		return true;

	if (strstr(pszSoundFile, "bhit_kevlar"))
		return true;

	if (strstr(pszSoundFile, "bhit_flesh"))
		return true;

	if (strstr(pszSoundFile, "die"))
		return true;

	if (strstr(pszSoundFile, "death"))
		return true;

	return false;
}

bool CSound::IsFilterSound(char* pszSoundFile)
{
	if (strstr(pszSoundFile, "c4_beep"))
		return false;

	if (strstr(pszSoundFile, "flashbang"))
		return false;

	if (strstr(pszSoundFile, "he_bounce"))
		return false;

	if (strstr(pszSoundFile, "grenade_hit"))
		return false;

	if (strstr(pszSoundFile, "sg_explode"))
		return false;

	if (strstr(pszSoundFile, "debris"))
		return false;

	return true;
}

void CSound::UpdateDormantPlayer(int index, Vector origin)
{
	if (g_Player[index]->m_bIsInPVS)
		return;

	const auto time = static_cast<float>(client_state->time);

	g_Player[index]->m_flUpdateIntervalTime = time - g_Player[index]->m_flHistory;
	g_Player[index]->m_flHistory = time;
	g_Player[index]->m_flDistance = g_Local->m_vecOrigin.Distance(origin);
	g_Player[index]->m_vecPrevOrigin = g_Player[index]->m_bSoundUpdated ? g_Player[index]->m_vecOrigin : origin;
	g_Player[index]->m_vecOrigin = origin;
	g_Player[index]->m_bIsDead = false;
	g_Player[index]->m_bSoundUpdated = true;

	if (g_Player[index]->m_iHealth <= 0)
		g_Player[index]->m_iHealth = Game::GetRespawnHealth();
}

void CSound::UpdatePlayerArmor(int index, char* pszSoundFile)
{
	if (strstr(pszSoundFile, "bhit_helmet")) 
		g_Player[index]->m_iArmorType = ARMOR_VESTHELM;

	if (strstr(pszSoundFile, "bhit_kevlar") && g_Player[index]->m_iArmorType != ARMOR_VESTHELM)
		g_Player[index]->m_iArmorType = ARMOR_KEVLAR;

	if (strstr(pszSoundFile, "bhit_flesh")) 
		g_Player[index]->m_iArmorType = ARMOR_NONE;

	if (strstr(pszSoundFile, "headshot"))
		g_Player[index]->m_iArmorType = ARMOR_NONE;
}

#define UpdateWeaponBySound(weapon)\
	if (strstr(pszSoundFile, weapon)) {\
		strcpy_s(g_Player[index]->m_szWeaponModelName, weapon);\
		return;\
}

void CSound::UpdatePlayerWeapon(int index, char* pszSoundFile)
{
	UpdateWeaponBySound("ak47");
	UpdateWeaponBySound("deagle");
	UpdateWeaponBySound("elite");
	UpdateWeaponBySound("famas");
	UpdateWeaponBySound("fiveseven");
	UpdateWeaponBySound("g3sg1");
	UpdateWeaponBySound("galil");
	UpdateWeaponBySound("glock18");
	UpdateWeaponBySound("knife");
	UpdateWeaponBySound("m3");
	UpdateWeaponBySound("m4a1");
	UpdateWeaponBySound("m249");
	UpdateWeaponBySound("mac10");
	UpdateWeaponBySound("mp5");
	UpdateWeaponBySound("p90");
	UpdateWeaponBySound("p228");
	UpdateWeaponBySound("scout");
	UpdateWeaponBySound("sg550");
	UpdateWeaponBySound("sg552");
	UpdateWeaponBySound("tmp");
	UpdateWeaponBySound("ump45");
	UpdateWeaponBySound("usp");
	UpdateWeaponBySound("xm1014");
}

void CSound::UpdatePlayerHealth(int index, char* pszSoundFile)
{
	int iDamage = 0;

	if (!g_HitRegister.RegisterHit(index, pszSoundFile))
	{
		UpdatePlayerArmor(index, pszSoundFile);

		if (strstr(pszSoundFile, "bhit_helmet"))
			iDamage = g_Engine.pfnRandomLong(65, 70);

		if (strstr(pszSoundFile, "bhit_kevlar"))
			iDamage = g_Engine.pfnRandomLong(15, 20);

		if (strstr(pszSoundFile, "bhit_flesh"))
			iDamage = g_Engine.pfnRandomLong(25, 30);

		if (strstr(pszSoundFile, "headshot"))
			iDamage = g_Engine.pfnRandomLong(75, 80);
	}
	
	if (iDamage <= g_Player[index]->m_iHealth)
		g_Player[index]->m_iHealth -= iDamage;

	if (strstr(pszSoundFile, "die") || strstr(pszSoundFile, "death"))
	{
		g_Player[index]->m_iArmorType = ARMOR_NONE;
		g_Player[index]->m_iHealth = 0;
	}
}

void CSound::GetResolvedIndex(int index, Vector origin, int& resolved)
{
	resolved = index;

	if (g_Entity[resolved]->m_bIsPlayer)
		return;

	for (auto& resolver : m_sound_resolver)
	{
		if (resolver.orig_index == index)
		{
			resolved = resolver.resolved_index;
			break;
		}
	}

	for (int i = 1; i <= client_state->maxclients; i++)
	{
		if (g_Player[i]->m_bIsLocal)
			continue;

		if (!g_Player[i]->m_bIsConnected)
			continue;

		if (!g_Player[i]->m_iMessageNum)
			continue;

		if (!g_Player[i]->m_iTeamNum)
			continue;

		if (!g_Player[i]->m_bIsInPVS)
			continue;

		if (g_Player[i]->m_bIsDead)
			continue;

		if (g_Player[i]->m_vecOrigin.Distance(origin) <= sqrt(2.f) * 16.f)
		{
			for (auto& sound : m_sound_resolver)
			{
				if (sound.orig_index == index)
				{
					sound.resolved_index = i;
					resolved = sound.resolved_index;
					return;
				}
			}

			m_sound_resolver.push_back(sound_resolver_data{ index, resolved = i });
			return;
		}
	}
}

void CSound::ClearResolvedPlayer(int index)
{
	for (size_t i = 0; i < m_sound_resolver.size(); i++)
	{
		if (m_sound_resolver[i].orig_index == index)
		{
			m_sound_resolver.erase(m_sound_resolver.begin() + i);
			return;
		}
	}
}

void CSound::ClearResolverData()
{
	if (!m_sound_resolver.empty())
		m_sound_resolver.clear();
}

std::unique_ptr<CSound> g_pSound;