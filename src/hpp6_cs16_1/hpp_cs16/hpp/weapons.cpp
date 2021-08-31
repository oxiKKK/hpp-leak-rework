#include "main.h"

CBasePlayerWeapon g_Weapon;

CCSPlayerWeapon CBasePlayerWeapon::GetCSWeapon(int iWeaponID)
{
	using GetCSWeapon_t = int*(__cdecl*)(int);

	auto GetCSWeaponFn = (GetCSWeapon_t)(g_Offsets.m_dwInitPoint);

	auto dwCSPlayerWeapon = GetCSWeaponFn(iWeaponID);

	if (dwCSPlayerWeapon)
		return *(CCSPlayerWeapon*)(dwCSPlayerWeapon);

	return CCSPlayerWeapon();
}

void CBasePlayerWeapon::ItemPreFrame(local_state_s* to, usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	if (runfuncs)
	{
		data.m_iRandomSeed = random_seed;
		data.m_iFlags = to->client.flags;
		data.m_iWeaponID = to->client.m_iId;
		data.m_iClip = to->weapondata[to->client.m_iId].m_iClip;
		data.m_flNextPrimaryAttack = to->weapondata[to->client.m_iId].m_flNextPrimaryAttack;
		data.m_flNextSecondaryAttack = to->weapondata[to->client.m_iId].m_flNextSecondaryAttack;
		data.m_iInReload = (to->weapondata[to->client.m_iId].m_fInReload || !to->weapondata[to->client.m_iId].m_iClip);
		data.m_iWeaponState = to->weapondata[to->client.m_iId].m_iWeaponState;
		data.m_flNextAttack = to->client.m_flNextAttack;
		data.m_iUser3 = to->client.iuser3;
	}
}

void CBasePlayerWeapon::Update()
{
	auto CSWeapon = GetCSWeapon(data.m_iWeaponID); // get data current weapon

	data.m_flAccuracy = CSWeapon.m_flAccuracy;
	data.m_iShotsFired = CSWeapon.m_iShotsFired;
	data.m_flTimeWeaponIdle = CSWeapon.m_flTimeWeaponIdle;
	data.m_flStartThrow = CSWeapon.m_flStartThrow;
	data.m_flReleaseThrow = CSWeapon.m_flReleaseThrow;
	//data.m_flNextPrimaryAttack = CSWeapon.m_flNextPrimaryAttack;

	_UpdateWeaponData();
}

void CBasePlayerWeapon::_UpdateWeaponData()
{
	switch (data.m_iWeaponID) 
	{
	case WEAPON_P228:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 4096.0f;
		data.m_flWallPierce1 = 0.8f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 32;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_357SIG;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 1.5 * (1 - data.m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.255 * (1 - data.m_flAccuracy);
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.075 * (1 - data.m_flAccuracy);
		else
			data.m_flSpread = 0.15 * (1 - data.m_flAccuracy);
		break;
	case WEAPON_SCOUT:
		data.m_iPenetration = WALL_PEN2;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.98f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 75;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_762MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 0.2;
		else if (g_Local.m_flVelocity > 170)
			data.m_flSpread = 0.075;
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0;
		else
			data.m_flSpread = 0.007;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			data.m_flSpread += 0.025f;
		break;
	case WEAPON_XM1014:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 3048.0f;
		data.m_flWallPierce1 = 0.0f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 4;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		data.m_flSpread = 0;
		break;
	case WEAPON_MAC10:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.82f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 29;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_45ACP;

		if (data.m_iFlags & FL_ONGROUND)
			data.m_flSpread = 0.03 * data.m_flAccuracy;
		else
			data.m_flSpread = 0.375 * data.m_flAccuracy;
		break;
	case WEAPON_AUG:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.96f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 32;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.4 + 0.035;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.07 + 0.035;
		else
			data.m_flSpread = data.m_flAccuracy * 0.02;
		break;
	case WEAPON_ELITE:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.75f; // left
		data.m_flWallPierce2 = 0.75f; // right
		data.m_iDamage1 = 20; // left
		data.m_iDamage2 = 27; // right
		data.m_iBulletType = BULLET_PLAYER_9MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 1.3 * (1 - data.m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.175 * (1 - data.m_flAccuracy);
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.08 * (1 - data.m_flAccuracy);
		else
			data.m_flSpread = 0.1 * (1 - data.m_flAccuracy);
		break;
	case WEAPON_FIVESEVEN:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 4096.0f;
		data.m_flWallPierce1 = 0.885f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 14;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_357SIG;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 1.5 * (1 - data.m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.255 * (1 - data.m_flAccuracy);
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.075 * (1 - data.m_flAccuracy);
		else
			data.m_flSpread = 0.15 * (1 - data.m_flAccuracy);
		break;
	case WEAPON_UMP45:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.82f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 30;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_45ACP;

		if (data.m_iFlags & FL_ONGROUND)
			data.m_flSpread = 0.04 * data.m_flAccuracy;
		else
			data.m_flSpread = 0.24 * data.m_flAccuracy;
		break;
	case WEAPON_SG550:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.98f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 40;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 0.45 * (1 - data.m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.15;
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.04 * (1 - data.m_flAccuracy);
		else
			data.m_flSpread = 0.05 * (1 - data.m_flAccuracy);

		if (g_Local.m_iFOV == DEFAULT_FOV)
			data.m_flSpread += 0.025f;
		break;
	case WEAPON_GALIL:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.96f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 33;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.3 + 0.04;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.07 + 0.04;
		else
			data.m_flSpread = data.m_flAccuracy * 0.0375;
		break;
	case WEAPON_FAMAS:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.96f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 33;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.3 + 0.030;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.07 + 0.030;
		else
			data.m_flSpread = data.m_flAccuracy * 0.02;

		if (!IsInBurst())
			data.m_flSpread += 0.01f;
		break;
	case WEAPON_USP:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 4096.0f;
		data.m_flWallPierce1 = 0.79f; // unsilenced
		data.m_flWallPierce2 = 0.79f; // silenced
		data.m_iDamage1 = 34; // unsilenced
		data.m_iDamage2 = 30; // silenced
		data.m_iBulletType = BULLET_PLAYER_45ACP;

		if (IsSilenced()) {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = 1.3 * (1 - data.m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				data.m_flSpread = 0.25 * (1 - data.m_flAccuracy);
			else if (data.m_iFlags & FL_DUCKING)
				data.m_flSpread = 0.125 * (1 - data.m_flAccuracy);
			else
				data.m_flSpread = 0.15 * (1 - data.m_flAccuracy);
		}
		else {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = 1.2 * (1 - data.m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				data.m_flSpread = 0.225 * (1 - data.m_flAccuracy);
			else if (data.m_iFlags & FL_DUCKING)
				data.m_flSpread = 0.08 * (1 - data.m_flAccuracy);
			else
				data.m_flSpread = 0.1 * (1 - data.m_flAccuracy);
		}
		break;
	case WEAPON_GLOCK18:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.75f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 20;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_9MM;

		if (IsInBurst()) {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = 1.2 * (1 - data.m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				data.m_flSpread = 0.185 * (1 - data.m_flAccuracy);
			else if (data.m_iFlags & FL_DUCKING)
				data.m_flSpread = 0.095 * (1 - data.m_flAccuracy);
			else
				data.m_flSpread = 0.3 * (1 - data.m_flAccuracy);
		}
		else {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = 1.0 * (1 - data.m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				data.m_flSpread = 0.165 * (1 - data.m_flAccuracy);
			else if (data.m_iFlags & FL_DUCKING)
				data.m_flSpread = 0.075 * (1 - data.m_flAccuracy);
			else
				data.m_flSpread = 0.1 * (1 - data.m_flAccuracy);
		}
		break;
	case WEAPON_AWP:
		data.m_iPenetration = WALL_PEN2;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.99f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 115;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_338MAG;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 0.85;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = 0.25;
		else if (g_Local.m_flVelocity > 10)
			data.m_flSpread = 0.1;
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.0;
		else
			data.m_flSpread = 0.001;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			data.m_flSpread += 0.08f;
		break;
	case WEAPON_MP5N:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.84f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 26;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_9MM;

		if (data.m_iFlags & FL_ONGROUND)
			data.m_flSpread = 0.04 * data.m_flAccuracy;
		else
			data.m_flSpread = 0.2 * data.m_flAccuracy;
		break;
	case WEAPON_M249:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.97f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 32;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.5 + 0.045;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.095 + 0.045;
		else
			data.m_flSpread = data.m_flAccuracy * 0.03;
		break;
	case WEAPON_M3:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 3000.0f;
		data.m_flWallPierce1 = 0.0f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 4;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		data.m_flSpread = 0.f;
		break;
	case WEAPON_M4A1:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.97f; // unsilenced
		data.m_flWallPierce2 = 0.95f; // silenced
		data.m_iDamage1 = 32; // unsilenced
		data.m_iDamage2 = 33; // silenced
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (IsSilenced()) {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = data.m_flAccuracy * 0.4 + 0.035;
			else if (g_Local.m_flVelocity > 140)
				data.m_flSpread = data.m_flAccuracy * 0.07 + 0.035;
			else
				data.m_flSpread = data.m_flAccuracy * 0.025;
		}
		else {
			if (!(data.m_iFlags & FL_ONGROUND))
				data.m_flSpread = data.m_flAccuracy * 0.4 + 0.035;
			else if (g_Local.m_flVelocity > 140)
				data.m_flSpread = data.m_flAccuracy * 0.07 + 0.035;
			else
				data.m_flSpread = data.m_flAccuracy * 0.02;
		}
		break;
	case WEAPON_TMP:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.85f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 20;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_9MM;

		if (data.m_iFlags & FL_ONGROUND)
			data.m_flSpread = 0.03 * data.m_flAccuracy;
		else
			data.m_flSpread = 0.25 * data.m_flAccuracy;
		break;
	case WEAPON_G3SG1:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.98f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 60;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_762MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 0.45;
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.15;
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.035;
		else
			data.m_flSpread = 0.055;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			data.m_flSpread += 0.025f;
		//vecDir = m_pPlayer->FireBullets3(vecSrc, vecAiming, (1 - m_flAccuracy) * flSpread, 8192, 3, BULLET_PLAYER_762MM, flBaseDamage, G3SG1_RANGE_MODIFER, m_pPlayer->pev, true, m_pPlayer->random_seed);
		data.m_flSpread = (1 - data.m_flAccuracy) * data.m_flSpread;
		break;
	case WEAPON_DEAGLE:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 4096.0f;
		data.m_flWallPierce1 = 0.81f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 54;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_50AE;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = 1.5 * (1 - data.m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			data.m_flSpread = 0.25 * (1 - data.m_flAccuracy);
		else if (data.m_iFlags & FL_DUCKING)
			data.m_flSpread = 0.115 * (1 - data.m_flAccuracy);
		else
			data.m_flSpread = 0.13 * (1 - data.m_flAccuracy);
		break;
	case WEAPON_SG552:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.955f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 33;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_556MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.45 + 0.035;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.075 + 0.035;
		else
			data.m_flSpread = data.m_flAccuracy * 0.02;
		break;
	case WEAPON_AK47:
		data.m_iPenetration = WALL_PEN1;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.98f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 36;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_762MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.4 + 0.04;
		else if (g_Local.m_flVelocity > 140)
			data.m_flSpread = data.m_flAccuracy * 0.07 + 0.04;
		else
			data.m_flSpread = data.m_flAccuracy * 0.0275;
		break;
	case WEAPON_P90:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 8192.0f;
		data.m_flWallPierce1 = 0.885f;
		data.m_flWallPierce2 = 0.0f;
		data.m_iDamage1 = 21;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_PLAYER_57MM;

		if (!(data.m_iFlags & FL_ONGROUND))
			data.m_flSpread = data.m_flAccuracy * 0.3;
		else if (g_Local.m_flVelocity > 170)
			data.m_flSpread = data.m_flAccuracy * 0.115;
		else
			data.m_flSpread = data.m_flAccuracy * 0.045;
		break;
	default:
		data.m_iPenetration = WALL_PEN0;
		data.m_flPenetrationDistance = 0;
		data.m_flWallPierce1 = 0;
		data.m_flWallPierce2 = 0;
		data.m_iDamage1 = 0;
		data.m_iDamage2 = 0;
		data.m_iBulletType = BULLET_NONE;
		data.m_flAccuracy = 0;
		data.m_flSpread = 0;
		break;
	}
}

bool CBasePlayerWeapon::IsFreezePeriod(void)
{
	if (data.m_iUser3 & PLAYER_FREEZE_TIME_OVER)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsSilenced(void)
{
	if (data.m_iWeaponID == WEAPON_M4A1 && data.m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return true;
	if (data.m_iWeaponID == WEAPON_USP && data.m_iWeaponState & WPNSTATE_USP_SILENCED)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsInBurst(void)
{
	if (data.m_iWeaponID == WEAPON_GLOCK18 && data.m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
		return true;
	if (data.m_iWeaponID == WEAPON_FAMAS && data.m_iWeaponState & WPNSTATE_FAMAS_BURST_MODE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsLeftElite(void)
{
	if (data.m_iWeaponID == WEAPON_ELITE && data.m_iWeaponState & WPNSTATE_ELITE_LEFT)
		return true;

	return false;
}

int CBasePlayerWeapon::GetPenetration(void)
{
	return data.m_iPenetration;
}

int CBasePlayerWeapon::GetBulletType(void)
{
	return data.m_iBulletType;
}

float CBasePlayerWeapon::GetDistance(void)
{
	return data.m_flPenetrationDistance;
}

float CBasePlayerWeapon::GetWallPierce(void)
{
	if (IsSilenced())
		return data.m_flWallPierce2;
	else
		return data.m_flWallPierce1;
}

int CBasePlayerWeapon::GetDamage(void)
{
	if (data.m_iWeaponID == WEAPON_ELITE) {
		if (data.m_iWeaponState & WPNSTATE_ELITE_LEFT)
			return data.m_iDamage1;
		else
			return data.m_iDamage2;
	}

	if (IsSilenced())
		return data.m_iDamage2;
	else
		return data.m_iDamage1;
}

bool CBasePlayerWeapon::IsKnife(void)
{
	if (data.m_iWeaponID == WEAPON_KNIFE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsSniper(void)
{
	switch (data.m_iWeaponID) {
	case WEAPON_AWP:
	case WEAPON_SCOUT:
	case WEAPON_G3SG1:
	case WEAPON_SG550:
		return true;
	}

	return false;
}

bool CBasePlayerWeapon::IsPistol(void)
{
	switch (data.m_iWeaponID) {
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

bool CBasePlayerWeapon::IsRifle(void)
{
	switch (data.m_iWeaponID) {
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

bool CBasePlayerWeapon::IsShotGun(void)
{
	if (data.m_iWeaponID == WEAPON_XM1014 || data.m_iWeaponID == WEAPON_M3)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsMachineGun(void)
{
	if (data.m_iWeaponID == WEAPON_M249)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsSubMachineGun(void)
{
	switch (data.m_iWeaponID) {
	case WEAPON_TMP:
	case WEAPON_P90:
	case WEAPON_MP5N:
	case WEAPON_MAC10:
	case WEAPON_UMP45:
		return true;
	}

	return false;
}

int CBasePlayerWeapon::GetClassType()
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

bool CBasePlayerWeapon::IsGun(void)
{
	if (!IsNonAttack() && !IsKnife())
		return true;

	return false;
}

bool CBasePlayerWeapon::IsNonAttack(void)
{
	if (data.m_iWeaponID == WEAPON_HEGRENADE || data.m_iWeaponID == WEAPON_FLASHBANG || data.m_iWeaponID == WEAPON_C4 || data.m_iWeaponID == WEAPON_SMOKEGRENADE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsNade(void)
{
	if (data.m_iWeaponID == WEAPON_HEGRENADE || data.m_iWeaponID == WEAPON_FLASHBANG || data.m_iWeaponID == WEAPON_SMOKEGRENADE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsC4(void)
{
	if (data.m_iWeaponID == WEAPON_C4)
		return true;

	return false;
}

bool CBasePlayerWeapon::CanAttack(void)
{
	if (data.m_flNextPrimaryAttack <= 0.f && data.m_flNextSecondaryAttack <= 0.f && !data.m_iInReload && (data.m_flNextAttack <= 0.f || data.m_flNextAttack > 20.f)) //m_flNextAttack bypass shit anticheats
		return true;

	return false;
}