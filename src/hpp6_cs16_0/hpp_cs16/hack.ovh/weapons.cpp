#include "main.h"

CBasePlayerWeapon g_Weapon;

void CBasePlayerWeapon::PrimaryAttack()
{
	switch (m_iWeaponID) {
	case WEAPON_AK47:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35f;

		if (m_flAccuracy > 1.25f)
			m_flAccuracy = 1.25f;
		break;
	case WEAPON_AUG:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 215) + 0.3f;

		if (m_flAccuracy > 1.0f)
			m_flAccuracy = 1.0f;
		break;
	case WEAPON_DEAGLE:
		if (++m_iShotsFired > 1)
			return;

		if (m_flPreviousTime != 0.0) {
			m_flAccuracy -= (0.4f - (m_flCurrentTime - m_flPreviousTime)) * 0.35f;

			if (m_flAccuracy > 0.9f)
				m_flAccuracy = 0.9f;
			else if (m_flAccuracy < 0.55f)
				m_flAccuracy = 0.55f;
		}
		break;
	case WEAPON_ELITE:
		if (++m_iShotsFired > 1)
			return;

		if (m_flPreviousTime) {
			m_flAccuracy -= (0.325f - m_flCurrentTime - m_flPreviousTime) * 0.275f;

			if (m_flAccuracy > 0.88f)
				m_flAccuracy = 0.88f;
			else if (m_flAccuracy < 0.55f)
				m_flAccuracy = 0.55f;
		}
		break;
	case WEAPON_FAMAS:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = (m_iShotsFired * m_iShotsFired * m_iShotsFired / 215) + 0.3f;

		if (m_flAccuracy > 1.0f)
			m_flAccuracy = 1.0f;
		break;
	case WEAPON_FIVESEVEN:
		if (++m_iShotsFired > 1)
			return;

		if (m_flPreviousTime != 0.0f) {
			m_flAccuracy -= (0.275f - (m_flCurrentTime - m_flPreviousTime)) * 0.25f;

			if (m_flAccuracy > 0.92f)
				m_flAccuracy = 0.92f;
			else if (m_flAccuracy < 0.725f) {
				m_flAccuracy = 0.725f;
			}
		}
		break;
	case WEAPON_G3SG1:
		if (m_flPreviousTime) {
			m_flAccuracy = (m_flCurrentTime - m_flPreviousTime) * 0.3f + 0.55f;

			if (m_flAccuracy > 0.98f)
				m_flAccuracy = 0.98f;
		}
		else
			m_flAccuracy = 0.98f;
		break;
	case WEAPON_GALIL:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.35f;

		if (m_flAccuracy > 1.25f)
			m_flAccuracy = 1.25f;
		break;
	case WEAPON_GLOCK18:
		if (!IsCurrentWeaponInBurst()) {
			if (++m_iShotsFired > 1)
				return;
		}

		if (m_flPreviousTime) {
			m_flAccuracy -= (0.325f - (m_flCurrentTime - m_flPreviousTime)) * 0.275f;

			if (m_flAccuracy > 0.9f)
				m_flAccuracy = 0.9f;
			else if (m_flAccuracy < 0.6f)
				m_flAccuracy = 0.6f;
		}
		break;
	case WEAPON_M4A1:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3f;

		if (m_flAccuracy > 1)
			m_flAccuracy = 1;
		break;
	case WEAPON_M249:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 175) + 0.4f;

		if (m_flAccuracy > 0.9f)
			m_flAccuracy = 0.9f;
		break;
	case WEAPON_MAC10:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.6f;

		if (m_flAccuracy > 1.65f)
			m_flAccuracy = 1.65f;
		break;
	case WEAPON_MP5N:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 220.1) + 0.45f;

		if (m_flAccuracy > 0.75f)
			m_flAccuracy = 0.75f;
		break;
	case WEAPON_P90:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = (m_iShotsFired * m_iShotsFired / 175) + 0.45f;

		if (m_flAccuracy > 1)
			m_flAccuracy = 1;
		break;
	case WEAPON_P228:
		if (++m_iShotsFired > 1)
			return;

		if (m_flPreviousTime != 0.0f) {
			m_flAccuracy -= (0.325f - (m_flCurrentTime - m_flPreviousTime)) * 0.3f;

			if (m_flAccuracy > 0.9f)
				m_flAccuracy = 0.9f;
			else if (m_flAccuracy < 0.6f)
				m_flAccuracy = 0.6f;
		}
		break;
	case WEAPON_SG550:
		if (m_flPreviousTime) {
			m_flAccuracy = (m_flCurrentTime - m_flPreviousTime) * 0.35f + 0.65f;

			if (m_flAccuracy > 0.98f)
				m_flAccuracy = 0.98f;
		}
		break;
	case WEAPON_SG552:
		m_bDelayFire = true;
		m_iShotsFired++;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 220) + 0.3f;

		if (m_flAccuracy > 1.0f)
			m_flAccuracy = 1.0f;
		break;
	case WEAPON_TMP:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired * m_iShotsFired) / 200) + 0.55f;

		if (m_flAccuracy > 1.4f)
			m_flAccuracy = 1.4f;
		break;
	case WEAPON_UMP45:
		m_bDelayFire = true;
		++m_iShotsFired;

		m_flAccuracy = ((m_iShotsFired * m_iShotsFired) / 210) + 0.5f;

		if (m_flAccuracy > 1.0f)
			m_flAccuracy = 1.0f;
		break;
	case WEAPON_USP:
		if (++m_iShotsFired > 1)
			return;

		if (m_flPreviousTime != 0.0f) {
			m_flAccuracy -= (0.3f - (m_flCurrentTime - m_flPreviousTime)) * 0.275f;

			if (m_flAccuracy > 0.92f)
				m_flAccuracy = 0.92f;
			else if (m_flAccuracy < 0.6f)
				m_flAccuracy = 0.6f;
		}
		break;
	}
	//g_Engine.Con_Printf((char*)"m_flAccuracy: %f | m_flLastFire: %f (%f) | m_iShotsFired: %i\n", g_Weapon.m_flAccuracy, g_Weapon.m_flPreviousTime, g_Weapon.m_flCurrentTime - g_Weapon.m_flPreviousTime, g_Weapon.m_iShotsFired);
	m_flPreviousTime = m_flCurrentTime;
}

void CBasePlayerWeapon::ItemPreFrame(struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed)
{
	if (runfuncs) 
	{
		m_iRandomSeed = random_seed;
		m_flCurrentTime = time;
		m_iFlags = to->client.flags;
		m_iWeaponID = to->client.m_iId;
		m_iClip = to->weapondata[to->client.m_iId].m_iClip;
		m_flNextPrimaryAttack = to->weapondata[to->client.m_iId].m_flNextPrimaryAttack;
		m_flNextSecondaryAttack = to->weapondata[to->client.m_iId].m_flNextSecondaryAttack;
		m_iInReload = (to->weapondata[to->client.m_iId].m_fInReload || !to->weapondata[to->client.m_iId].m_iClip);
		m_iWeaponState = to->weapondata[to->client.m_iId].m_iWeaponState;
		m_flNextAttack = to->client.m_flNextAttack;
		m_iUser3 = to->client.iuser3;
		m_vecPunchangle = to->client.punchangle;

		UpdateWeaponData();
	}
}

void CBasePlayerWeapon::SimulatePostFrame(struct usercmd_s* cmd)
{
	auto _m_flAccuracy = m_flAccuracy;
	auto _m_flSpread = m_flSpread;
	auto _m_flPreviousTime = m_flPreviousTime;
	auto _m_iShotsFired = m_iShotsFired;
	auto _m_bDelayFire = m_bDelayFire;

	PrimaryAttack();
	UpdateWeaponData();

	m_flSpreadSimulated = m_flSpread;
	m_flAccuracySimulated = m_flAccuracy;

	m_flSpread = _m_flSpread;
	m_flAccuracy = _m_flAccuracy;
	m_flPreviousTime = _m_flPreviousTime;
	m_iShotsFired = _m_iShotsFired;
	m_bDelayFire = _m_bDelayFire;
}

void CBasePlayerWeapon::ItemPostFrame(struct usercmd_s* cmd)
{
	static int iOldWeaponID = WEAPON_NONE;

	if (iOldWeaponID != m_iWeaponID)
	{
		Reset();
		iOldWeaponID = m_iWeaponID;
	}

	if (cmd->buttons & IN_ATTACK && CanAttack()) {
		// Can't shoot during the freeze period
		// Always allow firing in single player
		if (!IsFreezePeriod() && (/*m_pPlayer->m_bCanShoot && !m_pPlayer->m_bIsDefusing*/ m_iUser3 & PLAYER_CAN_SHOOT)) {
			PrimaryAttack();
		}
	}
	else if (m_iInReload || cmd->buttons & IN_RELOAD) {
		// reload when reload is pressed, or if no buttons are down and weapon is empty.
		Reset();//Reload();
	}
	else if (!(cmd->buttons & (IN_ATTACK | IN_ATTACK2))) {
		// no fire buttons down

		// The following code prevents the player from tapping the firebutton repeatedly
		// to simulate full auto and retaining the single shot accuracy of single fire
		if (m_bDelayFire) {
			m_bDelayFire = false;

			if (m_iShotsFired > 15) {
				m_iShotsFired = 15;
			}

			m_flDecreaseShotsFired = m_flCurrentTime + 0.4f;
		}

		// if it's a pistol then set the shots fired to 0 after the player releases a button
		if (IsCurrentWeaponPistol()) {
			m_iShotsFired = 0;
		}
		else {
			if (m_iShotsFired > 0 && m_flDecreaseShotsFired < m_flCurrentTime) {
				m_flDecreaseShotsFired = m_flDecreaseShotsFired + 0.0225f;
				m_iShotsFired--;
			}
		}
	}

	if ((!m_flAccuracy && m_flAccuracySimulated) || (!m_flSpread && m_flSpreadSimulated))
		g_Weapon.Reset();
}

void CBasePlayerWeapon::UpdateWeaponData()
{
	switch (m_iWeaponID) {
	case WEAPON_P228:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 4096.0f;
		m_flWallPierce1 = 0.8f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 32;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_357SIG;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 1.5 * (1 - m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.255 * (1 - m_flAccuracy);
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.075 * (1 - m_flAccuracy);
		else
			m_flSpread = 0.15 * (1 - m_flAccuracy);
		break;
	case WEAPON_SCOUT:
		m_iPenetration = WALL_PEN2;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.98f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 75;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_762MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 0.2;
		else if (g_Local.m_flVelocity > 170)
			m_flSpread = 0.075;
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0;
		else
			m_flSpread = 0.007;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			m_flSpread += 0.025f;
		break;
	case WEAPON_XM1014:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 3048.0f;
		m_flWallPierce1 = 0.0f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 4;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		m_flSpread = 0;
		break;
	case WEAPON_MAC10:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.82f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 29;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_45ACP;

		if (m_iFlags & FL_ONGROUND)
			m_flSpread = 0.03 * m_flAccuracy;
		else
			m_flSpread = 0.375 * m_flAccuracy;
		break;
	case WEAPON_AUG:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.96f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 32;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.4 + 0.035;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.07 + 0.035;
		else
			m_flSpread = m_flAccuracy * 0.02;
		break;
	case WEAPON_ELITE:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.75f; // left
		m_flWallPierce2 = 0.75f; // right
		m_iDamage1 = 20; // left
		m_iDamage2 = 27; // right
		m_iBulletType = BULLET_PLAYER_9MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 1.3 * (1 - m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.175 * (1 - m_flAccuracy);
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.08 * (1 - m_flAccuracy);
		else
			m_flSpread = 0.1 * (1 - m_flAccuracy);
		break;
	case WEAPON_FIVESEVEN:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 4096.0f;
		m_flWallPierce1 = 0.885f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 14;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_357SIG;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 1.5 * (1 - m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.255 * (1 - m_flAccuracy);
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.075 * (1 - m_flAccuracy);
		else
			m_flSpread = 0.15 * (1 - m_flAccuracy);
		break;
	case WEAPON_UMP45:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.82f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 30;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_45ACP;

		if (m_iFlags & FL_ONGROUND)
			m_flSpread = 0.04 * m_flAccuracy;
		else
			m_flSpread = 0.24 * m_flAccuracy;
		break;
	case WEAPON_SG550:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.98f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 40;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 0.45 * (1 - m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.15;
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.04 * (1 - m_flAccuracy);
		else
			m_flSpread = 0.05 * (1 - m_flAccuracy);

		if (g_Local.m_iFOV == DEFAULT_FOV)
			m_flSpread += 0.025f;
		break;
	case WEAPON_GALIL:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.96f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 33;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.3 + 0.04;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.07 + 0.04;
		else
			m_flSpread = m_flAccuracy * 0.0375;
		break;
	case WEAPON_FAMAS:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.96f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 33;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.3 + 0.030;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.07 + 0.030;
		else
			m_flSpread = m_flAccuracy * 0.02;

		if (!IsCurrentWeaponInBurst())
			m_flSpread += 0.01f;
		break;
	case WEAPON_USP:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 4096.0f;
		m_flWallPierce1 = 0.79f; // unsilenced
		m_flWallPierce2 = 0.79f; // silenced
		m_iDamage1 = 34; // unsilenced
		m_iDamage2 = 30; // silenced
		m_iBulletType = BULLET_PLAYER_45ACP;

		if (IsCurrentWeaponSilenced()) {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = 1.3 * (1 - m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				m_flSpread = 0.25 * (1 - m_flAccuracy);
			else if (m_iFlags & FL_DUCKING)
				m_flSpread = 0.125 * (1 - m_flAccuracy);
			else
				m_flSpread = 0.15 * (1 - m_flAccuracy);
		}
		else {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = 1.2 * (1 - m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				m_flSpread = 0.225 * (1 - m_flAccuracy);
			else if (m_iFlags & FL_DUCKING)
				m_flSpread = 0.08 * (1 - m_flAccuracy);
			else
				m_flSpread = 0.1 * (1 - m_flAccuracy);
		}
		break;
	case WEAPON_GLOCK18:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.75f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 20;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_9MM;

		if (IsCurrentWeaponInBurst()) {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = 1.2 * (1 - m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				m_flSpread = 0.185 * (1 - m_flAccuracy);
			else if (m_iFlags & FL_DUCKING)
				m_flSpread = 0.095 * (1 - m_flAccuracy);
			else
				m_flSpread = 0.3 * (1 - m_flAccuracy);
		}
		else {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = 1.0 * (1 - m_flAccuracy);
			else if (g_Local.m_flVelocity > 0)
				m_flSpread = 0.165 * (1 - m_flAccuracy);
			else if (m_iFlags & FL_DUCKING)
				m_flSpread = 0.075 * (1 - m_flAccuracy);
			else
				m_flSpread = 0.1 * (1 - m_flAccuracy);
		}
		break;
	case WEAPON_AWP:
		m_iPenetration = WALL_PEN2;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.99f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 115;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_338MAG;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 0.85;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = 0.25;
		else if (g_Local.m_flVelocity > 10)
			m_flSpread = 0.1;
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.0;
		else
			m_flSpread = 0.001;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			m_flSpread += 0.08f;
		break;
	case WEAPON_MP5N:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.84f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 26;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_9MM;

		if (m_iFlags & FL_ONGROUND)
			m_flSpread = 0.04 * m_flAccuracy;
		else
			m_flSpread = 0.2 * m_flAccuracy;
		break;
	case WEAPON_M249:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.97f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 32;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.5 + 0.045;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.095 + 0.045;
		else
			m_flSpread = m_flAccuracy * 0.03;
		break;
	case WEAPON_M3:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 3000.0f;
		m_flWallPierce1 = 0.0f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 4;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_BUCKSHOT;
		m_flSpread = 0.f;
		break;
	case WEAPON_M4A1:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.97f; // unsilenced
		m_flWallPierce2 = 0.95f; // silenced
		m_iDamage1 = 32; // unsilenced
		m_iDamage2 = 33; // silenced
		m_iBulletType = BULLET_PLAYER_556MM;

		if (IsCurrentWeaponSilenced()) {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = m_flAccuracy * 0.4 + 0.035;
			else if (g_Local.m_flVelocity > 140)
				m_flSpread = m_flAccuracy * 0.07 + 0.035;
			else
				m_flSpread = m_flAccuracy * 0.025;
		}
		else {
			if (!(m_iFlags & FL_ONGROUND))
				m_flSpread = m_flAccuracy * 0.4 + 0.035;
			else if (g_Local.m_flVelocity > 140)
				m_flSpread = m_flAccuracy * 0.07 + 0.035;
			else
				m_flSpread = m_flAccuracy * 0.02;
		}
		break;
	case WEAPON_TMP:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.85f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 20;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_9MM;

		if (m_iFlags & FL_ONGROUND)
			m_flSpread = 0.03 * m_flAccuracy;
		else
			m_flSpread = 0.25 * m_flAccuracy;
		break;
	case WEAPON_G3SG1:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.98f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 60;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_762MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 0.45;
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.15;
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.035;
		else
			m_flSpread = 0.055;

		if (g_Local.m_iFOV == DEFAULT_FOV)
			m_flSpread += 0.025f;
		break;
	case WEAPON_DEAGLE:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 4096.0f;
		m_flWallPierce1 = 0.81f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 54;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_50AE;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = 1.5 * (1 - m_flAccuracy);
		else if (g_Local.m_flVelocity > 0)
			m_flSpread = 0.25 * (1 - m_flAccuracy);
		else if (m_iFlags & FL_DUCKING)
			m_flSpread = 0.115 * (1 - m_flAccuracy);
		else
			m_flSpread = 0.13 * (1 - m_flAccuracy);
		break;
	case WEAPON_SG552:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.955f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 33;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_556MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.45 + 0.035;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.075 + 0.035;
		else
			m_flSpread = m_flAccuracy * 0.02;
		break;
	case WEAPON_AK47:
		m_iPenetration = WALL_PEN1;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.98f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 36;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_762MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.4 + 0.04;
		else if (g_Local.m_flVelocity > 140)
			m_flSpread = m_flAccuracy * 0.07 + 0.04;
		else
			m_flSpread = m_flAccuracy * 0.0275;
		break;
	case WEAPON_P90:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 8192.0f;
		m_flWallPierce1 = 0.885f;
		m_flWallPierce2 = 0.0f;
		m_iDamage1 = 21;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_PLAYER_57MM;

		if (!(m_iFlags & FL_ONGROUND))
			m_flSpread = m_flAccuracy * 0.3;
		else if (g_Local.m_flVelocity > 170)
			m_flSpread = m_flAccuracy * 0.115;
		else
			m_flSpread = m_flAccuracy * 0.045;
		break;
	default:
		m_iPenetration = WALL_PEN0;
		m_flPenetrationDistance = 0;
		m_flWallPierce1 = 0;
		m_flWallPierce2 = 0;
		m_iDamage1 = 0;
		m_iDamage2 = 0;
		m_iBulletType = BULLET_NONE;
		m_flAccuracy = 0;
		m_flSpread = 0;
		break;
	}
}

void CBasePlayerWeapon::Reset()
{
	switch (m_iWeaponID) {
	case WEAPON_AK47:
	case WEAPON_AUG:
	case WEAPON_FAMAS:
	case WEAPON_G3SG1:
	case WEAPON_GALIL:
	case WEAPON_M4A1:
	case WEAPON_M249:
	case WEAPON_P90:
	case WEAPON_SG552:
	case WEAPON_TMP:
		m_flAccuracy = 0.2f;
		break;
	case WEAPON_DEAGLE:
	case WEAPON_GLOCK18:
	case WEAPON_P228:
		m_flAccuracy = 0.9f;
		break;
	case WEAPON_ELITE:
		m_flAccuracy = 0.88f;
		break;
	case WEAPON_FIVESEVEN:
		m_flAccuracy = 0.92f;
		break;
	case WEAPON_MAC10:
		m_flAccuracy = 0.15f;
		break;
	case WEAPON_USP:
		m_flAccuracy = 0.92f;
		break;
	default:
		m_flAccuracy = 0;
		break;
	}

	m_iShotsFired = 0;
	m_bDelayFire = false;
	m_flPreviousTime = 0;
	m_flDecreaseShotsFired = m_flCurrentTime;
}

bool CBasePlayerWeapon::IsFreezePeriod(void)
{
	if (g_Weapon.m_iUser3 & PLAYER_FREEZE_TIME_OVER)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponSilenced(void)
{
	if (m_iWeaponID == WEAPON_M4A1 && m_iWeaponState & WPNSTATE_M4A1_SILENCED)
		return true;
	if (m_iWeaponID == WEAPON_USP && m_iWeaponState & WPNSTATE_USP_SILENCED)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponInBurst(void)
{
	if (m_iWeaponID == WEAPON_GLOCK18 && m_iWeaponState & WPNSTATE_GLOCK18_BURST_MODE)
		return true;
	if (m_iWeaponID == WEAPON_FAMAS && m_iWeaponState & WPNSTATE_FAMAS_BURST_MODE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponLeftElite(void)
{
	if (m_iWeaponID == WEAPON_ELITE && m_iWeaponState & WPNSTATE_ELITE_LEFT)
		return true;

	return false;
}

int CBasePlayerWeapon::CurrentPenetration(void)
{
	return m_iPenetration;
}

int CBasePlayerWeapon::CurrentBulletType(void)
{
	return m_iBulletType;
}

float CBasePlayerWeapon::CurrentDistance(void)
{
	return m_flPenetrationDistance;
}

float CBasePlayerWeapon::CurrentWallPierce(void)
{
	if (IsCurrentWeaponSilenced())
		return m_flWallPierce2;
	else
		return m_flWallPierce1;
}

int CBasePlayerWeapon::CurrentDamage(void)
{
	if (m_iWeaponID == WEAPON_ELITE) {
		if (m_iWeaponState & WPNSTATE_ELITE_LEFT)
			return m_iDamage1;
		else
			return m_iDamage2;
	}

	if (IsCurrentWeaponSilenced())
		return m_iDamage2;
	else
		return m_iDamage1;
}

bool CBasePlayerWeapon::IsCurrentWeaponKnife(void)
{
	if (m_iWeaponID == WEAPON_KNIFE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponSniper(void)
{
	switch (m_iWeaponID) {
	case WEAPON_AWP:
	case WEAPON_SCOUT:
	case WEAPON_G3SG1:
	case WEAPON_SG550:
		return true;
	}

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponPistol(void)
{
	switch (m_iWeaponID) {
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

bool CBasePlayerWeapon::IsCurrentWeaponRifle(void)
{
	switch (m_iWeaponID) {
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

bool CBasePlayerWeapon::IsCurrentWeaponShotGun(void)
{
	if (m_iWeaponID == WEAPON_XM1014 || m_iWeaponID == WEAPON_M3)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponMachineGun(void)
{
	if (m_iWeaponID == WEAPON_M249)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponSubMachineGun(void)
{
	switch (m_iWeaponID) {
	case WEAPON_TMP:
	case WEAPON_P90:
	case WEAPON_MP5N:
	case WEAPON_MAC10:
	case WEAPON_UMP45:
		return true;
	}

	return false;
}

int CBasePlayerWeapon::CurrentWeaponClassType()
{
	if (IsCurrentWeaponKnife())
		return WEAPONCLASS_KNIFE;

	if (IsCurrentWeaponPistol())
		return WEAPONCLASS_PISTOL;

	if (IsCurrentWeaponNade())
		return WEAPONCLASS_GRENADE;

	if (IsCurrentWeaponSniper())
		return WEAPONCLASS_SNIPERRIFLE;

	if (IsCurrentWeaponRifle())
		return WEAPONCLASS_RIFLE;

	if (IsCurrentWeaponShotGun())
		return WEAPONCLASS_SHOTGUN;

	if (IsCurrentWeaponMachineGun())
		return WEAPONCLASS_MACHINEGUN;

	if (IsCurrentWeaponSubMachineGun())
		return WEAPONCLASS_SUBMACHINEGUN;

	return WEAPONCLASS_NONE;
}

bool CBasePlayerWeapon::IsCurrentWeaponGun(void)
{
	if (!IsCurrentWeaponNonAttack() && !IsCurrentWeaponKnife())
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponNonAttack(void)
{
	if (m_iWeaponID == WEAPON_HEGRENADE || m_iWeaponID == WEAPON_FLASHBANG || m_iWeaponID == WEAPON_C4 || m_iWeaponID == WEAPON_SMOKEGRENADE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponNade(void)
{
	if (m_iWeaponID == WEAPON_HEGRENADE || m_iWeaponID == WEAPON_FLASHBANG || m_iWeaponID == WEAPON_SMOKEGRENADE)
		return true;

	return false;
}

bool CBasePlayerWeapon::IsCurrentWeaponC4(void)
{
	if (m_iWeaponID == WEAPON_C4)
		return true;

	return false;
}

bool CBasePlayerWeapon::CanAttack(void)
{
	if (m_flNextPrimaryAttack <= 0.f && g_Weapon.m_flNextSecondaryAttack <= 0.f && !m_iInReload && (m_flNextAttack <= 0.f || m_flNextAttack > 20.f)) //m_flNextAttack bypass shit anticheats
		return true;

	return false;
}