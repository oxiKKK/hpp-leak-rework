#include "framework.h"

void CBaseWeapon::AllocateMemory()
{
	if (!pBaseInfo.get())
		pBaseInfo = std::make_unique<CBaseWpnInfo>();
}

void CBaseWeapon::FreeMemory()
{
	DELETE_UNIQUE_PTR(pBaseInfo);
}

CCSPlayerWeapon* CBaseWeapon::GetCSWeaponData(int iWeaponID)
{
	if (iWeaponID == WEAPON_NONE)
		iWeaponID = pBaseInfo->m_iWeaponID;

	if (iWeaponID)
	{
		using GetCSWeapon_t = int* (__cdecl*)(int);

		const auto GetCSWeaponFn = (GetCSWeapon_t)(g_pGlobals->m_dwInitPoint);

		if (GetCSWeaponFn)
		{
			const auto CSPlayerWeapon = (CCSPlayerWeapon*)GetCSWeaponFn(iWeaponID);

			if (CSPlayerWeapon)
				return CSPlayerWeapon;
		}
	}

	return nullptr;
}

CBaseWpnInfo* CBaseWeapon::operator->()
{
	return pBaseInfo.get();
}

void CBaseWeapon::Update(local_state_s* from, local_state_s* to, usercmd_s* cmd, double time, unsigned random_seed)
{
	pBaseInfo->m_iWeaponID = to->client.m_iId;
	pBaseInfo->m_iWeaponState = to->weapondata[pBaseInfo->m_iWeaponID].m_iWeaponState;
	pBaseInfo->m_iClip = to->weapondata[pBaseInfo->m_iWeaponID].m_iClip;
	pBaseInfo->m_iFlags = to->client.flags;
	pBaseInfo->m_iUser1 = to->client.iuser1;
	pBaseInfo->m_iUser2 = to->client.iuser2;
	pBaseInfo->m_iUser3 = to->client.iuser3;
	pBaseInfo->m_iUser4 = to->client.iuser4;
	pBaseInfo->m_fInReload = to->weapondata[pBaseInfo->m_iWeaponID].m_fInReload;
	pBaseInfo->m_iRandomSeed = random_seed;

	pBaseInfo->m_flNextPrimaryAttack = to->weapondata[pBaseInfo->m_iWeaponID].m_flNextPrimaryAttack;
	pBaseInfo->m_flNextSecondaryAttack = to->weapondata[pBaseInfo->m_iWeaponID].m_flNextSecondaryAttack;
	pBaseInfo->m_flNextAttack = to->client.m_flNextAttack;
	
	pBaseInfo->m_dbTime = time;

	CCSPlayerWeapon* pWeapon = GetCSWeaponData();

	if (pWeapon)
	{
		pBaseInfo->m_flAccuracy = pWeapon->m_flAccuracy;
		pBaseInfo->m_flStartThrow = pWeapon->m_flStartThrow;
		pBaseInfo->m_flReleaseThrow = pWeapon->m_flReleaseThrow;
	}
	else
	{
		pBaseInfo->m_flAccuracy = 0.f;
		pBaseInfo->m_flStartThrow = 0.f;
		pBaseInfo->m_flReleaseThrow = 0.f;
	}

	UpdateShotsFired(cmd);

	switch (pBaseInfo->m_iWeaponID)
	{
	case WEAPON_P228:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 4096.f;
		pBaseInfo->m_flWallPierce1 = 0.8f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 32;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_357SIG;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(1.5 * (1 - pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.255 * (1 - pBaseInfo->m_flAccuracy));
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.075 * (1 - pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.15 * (1 - pBaseInfo->m_flAccuracy));
		break;
	case WEAPON_SCOUT:
		pBaseInfo->m_iPenetration = WALL_PEN2;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.98f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 75;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_762MM;

		if (!(pBaseInfo->m_iFlags & FL_ONGROUND))
			pBaseInfo->m_flSpread = float(0.2);
		else if (g_Local->m_flVelocity > 170.f)
			pBaseInfo->m_flSpread = float(0.075);
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = 0.f;
		else
			pBaseInfo->m_flSpread = float(0.007);

		if (g_Local->m_iFOV == DEFAULT_FOV)
			pBaseInfo->m_flSpread += 0.025f;
		break;
	case WEAPON_XM1014:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 3048.f;
		pBaseInfo->m_flWallPierce1 = 0.f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 20;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		pBaseInfo->m_flSpread = 0.f;
		break;
	case WEAPON_MAC10:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.82f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 29;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_45ACP;

		if (pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.03 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.375 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_AUG:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.96f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 32;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.035 + (0.4 * pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.035 + (0.07 * pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.02 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_ELITE:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.75f;
		pBaseInfo->m_flWallPierce2 = 0.75f;
		pBaseInfo->m_iDamage1 = 20;
		pBaseInfo->m_iDamage2 = 27;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_9MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(1.3 * (1 - pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.175 * (1 - pBaseInfo->m_flAccuracy));
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.08 * (1 - pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.1 * (1 - pBaseInfo->m_flAccuracy));
		break;
	case WEAPON_FIVESEVEN:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 4096.f;
		pBaseInfo->m_flWallPierce1 = 0.885f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 20;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_57MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(1.5 * (1 - pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.255 * (1 - pBaseInfo->m_flAccuracy));
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.075 * (1 - pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.15 * (1 - pBaseInfo->m_flAccuracy));
		break;
	case WEAPON_UMP45:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.82f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 30;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_45ACP;

		if (pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.04 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.24 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_SG550:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.98f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 70;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.45 * (1 - pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.15);
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.04 * (1 - pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.05 * (1 - pBaseInfo->m_flAccuracy));

		if (g_Local->m_iFOV == DEFAULT_FOV)
			pBaseInfo->m_flSpread += 0.025f;
		break;
	case WEAPON_GALIL:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.98f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 30;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.04 + (0.3 * pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.04 + (0.07 * pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.0375 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_FAMAS:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.96f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 30;
		pBaseInfo->m_iDamage2 = 34;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.03 + 0.3 * pBaseInfo->m_flAccuracy);
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.03 + 0.07 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.02 * pBaseInfo->m_flAccuracy);

		if (!IsInBurst())
			pBaseInfo->m_flSpread += 0.01f;
		break;
	case WEAPON_USP:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 4096.f;
		pBaseInfo->m_flWallPierce1 = 0.79f;
		pBaseInfo->m_flWallPierce2 = 0.79f;
		pBaseInfo->m_iDamage1 = 34;
		pBaseInfo->m_iDamage2 = 30;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_45ACP;

		if (IsSilenced()) 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(1.3 * (1 - pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 0.f)
				pBaseInfo->m_flSpread = float(0.25 * (1 - pBaseInfo->m_flAccuracy));
			else if (pBaseInfo->m_iFlags & FL_DUCKING)
				pBaseInfo->m_flSpread = float(0.125 * (1 - pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.15 * (1 - pBaseInfo->m_flAccuracy));
		}
		else 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(1.2 * (1 - pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 0.f)
				pBaseInfo->m_flSpread = float(0.225 * (1 - pBaseInfo->m_flAccuracy));
			else if (pBaseInfo->m_iFlags & FL_DUCKING)
				pBaseInfo->m_flSpread = float(0.08 * (1 - pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.1 * (1 - pBaseInfo->m_flAccuracy));
		}
		break;
	case WEAPON_GLOCK18:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.75f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 25;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_9MM;

		if (IsInBurst()) 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(1.2 * (1 - pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 0.f)
				pBaseInfo->m_flSpread = float(0.185 * (1 - pBaseInfo->m_flAccuracy));
			else if (pBaseInfo->m_iFlags & FL_DUCKING)
				pBaseInfo->m_flSpread = float(0.095 * (1 - pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.3 * (1 - pBaseInfo->m_flAccuracy));
		}
		else 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(1.0 * (1 - pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 0.f)
				pBaseInfo->m_flSpread = float(0.165 * (1 - pBaseInfo->m_flAccuracy));
			else if (pBaseInfo->m_iFlags & FL_DUCKING)
				pBaseInfo->m_flSpread = float(0.075 * (1 - pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.1 * (1 - pBaseInfo->m_flAccuracy));
		}
		break;
	case WEAPON_AWP:
		pBaseInfo->m_iPenetration = WALL_PEN2;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.99f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 115;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_338MAG;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.85);
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.25);
		else if (g_Local->m_flVelocity > 10.f)
			pBaseInfo->m_flSpread = float(0.1);
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = 0.f;
		else
			pBaseInfo->m_flSpread = float(0.001);

		if (g_Local->m_iFOV == DEFAULT_FOV)
			pBaseInfo->m_flSpread += 0.08f;
		break;
	case WEAPON_MP5N:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.84f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 26;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_9MM;

		if (pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.04 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.2 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_M249:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.97f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 32;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.045 + (0.5 * pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.045 + (0.095 * pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.03 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_M3:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 3000.f;
		pBaseInfo->m_flWallPierce1 = 0.f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 20;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		pBaseInfo->m_flSpread = 0.f;
		break;
	case WEAPON_M4A1:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.97f;
		pBaseInfo->m_flWallPierce2 = 0.95f;
		pBaseInfo->m_iDamage1 = 32;
		pBaseInfo->m_iDamage2 = 33;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (IsSilenced()) 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(0.035 + (0.4 * pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 140.f)
				pBaseInfo->m_flSpread = float(0.035 + (0.07 * pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.025 * pBaseInfo->m_flAccuracy);
		}
		else 
		{
			if (~pBaseInfo->m_iFlags & FL_ONGROUND)
				pBaseInfo->m_flSpread = float(0.035 + (0.4 * pBaseInfo->m_flAccuracy));
			else if (g_Local->m_flVelocity > 140.f)
				pBaseInfo->m_flSpread = float(0.035 + (0.07 * pBaseInfo->m_flAccuracy));
			else
				pBaseInfo->m_flSpread = float(0.02 * pBaseInfo->m_flAccuracy);
		}
		break;
	case WEAPON_TMP:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.85f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 20;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_9MM;

		if (pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.03 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.25 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_G3SG1:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.98f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 80;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_762MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.45);
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.15);
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.035);
		else
			pBaseInfo->m_flSpread = float(0.055);

		if (g_Local->m_iFOV == DEFAULT_FOV)
			pBaseInfo->m_flSpread += 0.025f;

		pBaseInfo->m_flSpread = (1 - pBaseInfo->m_flAccuracy) * pBaseInfo->m_flSpread;
		break;
	case WEAPON_DEAGLE:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 4096.f;
		pBaseInfo->m_flWallPierce1 = 0.81f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 54;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_50AE;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(1.5 * (1 - pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 0.f)
			pBaseInfo->m_flSpread = float(0.25 * (1 - pBaseInfo->m_flAccuracy));
		else if (pBaseInfo->m_iFlags & FL_DUCKING)
			pBaseInfo->m_flSpread = float(0.115 * (1 - pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.13 * (1 - pBaseInfo->m_flAccuracy));
		break;
	case WEAPON_SG552:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.955f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 33;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_556MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.035 + (0.45 * pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.035 + (0.075 * pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.02 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_AK47:
		pBaseInfo->m_iPenetration = WALL_PEN1;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.98f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 36;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_762MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.04 + (0.4 * pBaseInfo->m_flAccuracy));
		else if (g_Local->m_flVelocity > 140.f)
			pBaseInfo->m_flSpread = float(0.04 + (0.07 * pBaseInfo->m_flAccuracy));
		else
			pBaseInfo->m_flSpread = float(0.0275 * pBaseInfo->m_flAccuracy);
		break;
	case WEAPON_P90:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 8192.f;
		pBaseInfo->m_flWallPierce1 = 0.885f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 21;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_PLAYER_57MM;

		if (~pBaseInfo->m_iFlags & FL_ONGROUND)
			pBaseInfo->m_flSpread = float(0.3 * pBaseInfo->m_flAccuracy);
		else if (g_Local->m_flVelocity > 170.f)
			pBaseInfo->m_flSpread = float(0.115 * pBaseInfo->m_flAccuracy);
		else
			pBaseInfo->m_flSpread = float(0.045 * pBaseInfo->m_flAccuracy);
		break;
	default:
		pBaseInfo->m_iPenetration = WALL_PEN0;
		pBaseInfo->m_flDistance = 0.f;
		pBaseInfo->m_flWallPierce1 = 0.f;
		pBaseInfo->m_flWallPierce2 = 0.f;
		pBaseInfo->m_iDamage1 = 0;
		pBaseInfo->m_iDamage2 = 0;
		pBaseInfo->m_iBulletType = BULLET_NONE;
		pBaseInfo->m_flAccuracy = 0.f;
		pBaseInfo->m_flSpread = 0.f;
	}

	pBaseInfo->m_iAccuracyFlags = GetWeaponAccuracyFlags();
}

int CBaseWeapon::GetWeaponAccuracyFlags()
{
	switch (pBaseInfo->m_iWeaponID)
	{
	case WEAPON_AUG:
	case WEAPON_GALIL:
	case WEAPON_M249:
	case WEAPON_SG552:
	case WEAPON_AK47:
	case WEAPON_P90:
		return ACCURACY_AIR | ACCURACY_SPEED;
	case WEAPON_P228:
	case WEAPON_FIVESEVEN:
	case WEAPON_DEAGLE:
		return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
	case WEAPON_GLOCK18:
		if (IsInBurst())
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK | ACCURACY_MULTIPLY_BY_14_2;
		else
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
	case WEAPON_MAC10:
	case WEAPON_UMP45:
	case WEAPON_MP5N:
	case WEAPON_TMP:
		return ACCURACY_AIR;
	case WEAPON_M4A1:
		if (IsSilenced())
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_MULTIPLY_BY_14;
		else
			return ACCURACY_AIR | ACCURACY_SPEED;
	case WEAPON_FAMAS:
		if (IsInBurst())
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_MULTIPLY_BY_14_2;
		else
			return ACCURACY_AIR | ACCURACY_SPEED;
	case WEAPON_USP:
		if (IsSilenced())
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK | ACCURACY_MULTIPLY_BY_14;
		else
			return ACCURACY_AIR | ACCURACY_SPEED | ACCURACY_DUCK;
	}

	return NULL;
}

void CBaseWeapon::UpdateShotsFired(usercmd_s* cmd)
{
	static int iLastShotsFired;

	auto CalcShotsFired = [&](usercmd_s* cmd)
	{
		static int iShotsFired, iSaveClip;

		if (cmd->buttons & IN_ATTACK)
		{
			if (!iShotsFired && iSaveClip - pBaseInfo->m_iClip < 0)
				iSaveClip = pBaseInfo->m_iClip + 1;

			iShotsFired = iSaveClip - pBaseInfo->m_iClip;
		}
		else
		{
			iSaveClip = pBaseInfo->m_iClip;
			iShotsFired = 0;
		}

		return iShotsFired;
	};

	CCSPlayerWeapon* pWeapon = GetCSWeaponData();

	if (pWeapon)
	{
		if (pBaseInfo->m_iWeaponID == WEAPON_GLOCK18 && IsInBurst())
		{
			pBaseInfo->m_iShotsFired = pWeapon->m_iGlock18ShotsFired;
		}
		else if (pBaseInfo->m_iWeaponID == WEAPON_FAMAS && IsInBurst())
		{
			pBaseInfo->m_iShotsFired = pWeapon->m_iFamasShotsFired;
		}
		else if (!IsShotGun())
		{
			if (pWeapon->m_iShotsFired - iLastShotsFired < 2)
			{
				pBaseInfo->m_iShotsFired = pWeapon->m_iShotsFired;
				iLastShotsFired = pBaseInfo->m_iShotsFired;
			}
		}
		else
			pBaseInfo->m_iShotsFired = CalcShotsFired(cmd);
	}
	else
		pBaseInfo->m_iShotsFired = 0;
}

bool CBaseWeapon::IsSilenced()
{
	if (pBaseInfo->m_iWeaponID == WEAPON_M4A1 && pBaseInfo->m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return true;

	if (pBaseInfo->m_iWeaponID == WEAPON_USP && pBaseInfo->m_iWeaponState & WPNSTATE_USP_SILENCED)
		return true;

	return false;
}

bool CBaseWeapon::IsInBurst()
{
	if (pBaseInfo->m_iWeaponID == WEAPON_GLOCK18 && pBaseInfo->m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
		return true;

	if (pBaseInfo->m_iWeaponID == WEAPON_FAMAS && pBaseInfo->m_iWeaponState & WPNSTATE_FAMAS_BURST_MODE)
		return true;

	return false;
}

bool CBaseWeapon::IsLeftElite()
{
	return pBaseInfo->m_iWeaponID == WEAPON_ELITE && pBaseInfo->m_iWeaponState & WPNSTATE_ELITE_LEFT;
}

float CBaseWeapon::GetWallPierce()
{
	return IsSilenced() ? pBaseInfo->m_flWallPierce2 : pBaseInfo->m_flWallPierce1;
}

int CBaseWeapon::GetDamage()
{
	return (IsSilenced() || IsInBurst()) ? pBaseInfo->m_iDamage2 : pBaseInfo->m_iDamage1;
}

bool CBaseWeapon::IsKnife()
{
	return pBaseInfo->m_iWeaponID == WEAPON_KNIFE;
}

bool CBaseWeapon::IsSniper()
{
	switch (pBaseInfo->m_iWeaponID) 
	{
	case WEAPON_AWP:
	case WEAPON_SCOUT:
	case WEAPON_G3SG1:
	case WEAPON_SG550:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsPistol()
{
	switch (pBaseInfo->m_iWeaponID) 
	{
	case WEAPON_GLOCK18:
	case WEAPON_USP:
	case WEAPON_P228:
	case WEAPON_DEAGLE:
	case WEAPON_ELITE:
	case WEAPON_FIVESEVEN:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsRifle()
{
	switch (pBaseInfo->m_iWeaponID) 
	{
	case WEAPON_M4A1:
	case WEAPON_GALIL:
	case WEAPON_FAMAS:
	case WEAPON_AUG:
	case WEAPON_AK47:
	case WEAPON_SG552:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsShotGun()
{
	switch (pBaseInfo->m_iWeaponID)
	{
	case WEAPON_XM1014:
	case WEAPON_M3:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsMachineGun()
{
	return pBaseInfo->m_iWeaponID == WEAPON_M249;
}

bool CBaseWeapon::IsSubMachineGun()
{
	switch (pBaseInfo->m_iWeaponID) 
	{
	case WEAPON_TMP:
	case WEAPON_P90:
	case WEAPON_MP5N:
	case WEAPON_MAC10:
	case WEAPON_UMP45:
		return true;
	}

	return false;
}

int CBaseWeapon::GetClassType()
{
	if (IsKnife())
		return WEAPONCLASS_KNIFE;

	if (IsPistol())
		return WEAPONCLASS_PISTOL;

	if (IsNade())
		return WEAPONCLASS_GRENADE;

	if (IsSniper())
		return WEAPONCLASS_SNIPERRIFLE;

	if (IsRifle())
		return WEAPONCLASS_RIFLE;

	if (IsShotGun())
		return WEAPONCLASS_SHOTGUN;

	if (IsMachineGun())
		return WEAPONCLASS_MACHINEGUN;

	if (IsSubMachineGun())
		return WEAPONCLASS_SUBMACHINEGUN;

	return WEAPONCLASS_NONE;
}

bool CBaseWeapon::IsGun()
{
	return IsPistol() || IsSniper() || IsRifle() || IsShotGun() || IsMachineGun() || IsSubMachineGun();
}

bool CBaseWeapon::IsNonAttack()
{
	switch (pBaseInfo->m_iWeaponID)
	{
	case WEAPON_HEGRENADE:
	case WEAPON_FLASHBANG:
	case WEAPON_SMOKEGRENADE:
	case WEAPON_C4:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsNade()
{
	switch (pBaseInfo->m_iWeaponID)
	{
	case WEAPON_HEGRENADE:
	case WEAPON_FLASHBANG:
	case WEAPON_SMOKEGRENADE:
		return true;
	}

	return false;
}

bool CBaseWeapon::IsC4()
{
	return pBaseInfo->m_iWeaponID == WEAPON_C4;
}

bool CBaseWeapon::CanAttack()
{
	return pBaseInfo->m_flNextPrimaryAttack < 0.f && pBaseInfo->m_flNextSecondaryAttack < 0.f && pBaseInfo->m_fInReload <= 0.f && pBaseInfo->m_iClip && pBaseInfo->m_flNextAttack <= 0.f;
}

CBaseWeapon g_Weapon;