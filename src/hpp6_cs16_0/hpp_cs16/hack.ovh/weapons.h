enum Penerations
{
	WALL_PEN0,
	WALL_PEN1,
	WALL_PEN2,
};

class CBasePlayerWeapon;
class CBasePlayerWeapon {
private:

public:
	unsigned int m_iRandomSeed;

	float m_flPreviousTime;
	double m_flCurrentTime;

	int m_iWeaponState;
	int m_iFlags;
	int m_iClip;
	int m_iWeaponID;
	int m_iInReload;
	int m_iPenetration;
	int m_iBulletType;
	int m_iDamage1;
	int m_iDamage2;
	int m_iShotsFired;
	int m_iUser3;

	bool m_bDelayFire;

	float m_flDecreaseShotsFired;
	float m_flAccuracy;
	float m_flSpread;
	float m_flNextPrimaryAttack;
	float m_flNextSecondaryAttack;
	float m_flNextAttack;
	float m_flPenetrationDistance;
	float m_flWallPierce1;
	float m_flWallPierce2;
	float m_flAccuracySimulated;
	float m_flSpreadSimulated;

	Vector m_vecPunchangle;

	//Functions
	void ItemPreFrame(struct local_state_s* to, struct usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
	void ItemPostFrame(struct usercmd_s* cmd);
	void SimulatePostFrame(struct usercmd_s* cmd);
	void UpdateWeaponData();
	void PrimaryAttack();
	void Reset();

	bool CanAttack(void);
	bool IsCurrentWeaponGun(void);
	bool IsCurrentWeaponKnife(void);
	bool IsCurrentWeaponNonAttack(void);
	bool IsCurrentWeaponNade(void);
	bool IsCurrentWeaponC4(void);
	bool IsCurrentWeaponSniper(void);
	bool IsCurrentWeaponPistol(void);
	bool IsCurrentWeaponRifle(void);
	bool IsCurrentWeaponShotGun(void);
	bool IsCurrentWeaponMachineGun(void);
	bool IsCurrentWeaponSubMachineGun(void);
	bool IsCurrentWeaponSilenced(void);
	bool IsCurrentWeaponInBurst(void);
	bool IsCurrentWeaponLeftElite(void);
	bool IsFreezePeriod(void);

	int CurrentWeaponClassType(void);
	int CurrentPenetration(void);
	int CurrentBulletType(void);
	int CurrentDamage(void);
	float CurrentWallPierce(void);
	float CurrentDistance(void);
};
extern CBasePlayerWeapon g_Weapon;