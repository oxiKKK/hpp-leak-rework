#define ACCURACY_AIR (1 << 0) // accuracy depends on FL_ONGROUND
#define ACCURACY_SPEED (1 << 1)
#define ACCURACY_DUCK (1 << 2) // more accurate when ducking
#define ACCURACY_MULTIPLY_BY_14 (1 << 3) // accuracy multiply to 1.4
#define ACCURACY_MULTIPLY_BY_14_2 (1 << 4) // accuracy multiply to 1.4

enum Penerations
{
	WALL_PEN0,
	WALL_PEN1,
	WALL_PEN2,
};

class CCSPlayerWeapon
{
public:
	char	pad_0x0000[0x78];
	float	m_flStartThrow;
	float	m_flReleaseThrow;
	int		m_iSwing;
	char	pad_0x0084[0x28];
	int		m_iId;
	int		m_iPlayEmptySound;
	int		m_fFireOnEmpty;
	float	m_flNextPrimaryAttack;
	float	m_flNextSecondaryAttack;
	float	m_flTimeWeaponIdle;
	int		m_iPrimaryAmmoType;
	int		m_iSecondaryAmmoType;
	int		m_iClip;
	int		m_iClientClip;
	int		m_iClientWeaponState;
	int		m_fInReload;
	int		m_fInSpecialReload; 
	int		m_iDefaultAmmo;
	int		m_iShellId;
	float	m_fMaxSpeed;
	bool	m_bDelayFire;
	int		m_iDirection;
	bool	m_bSecondarySilencerOn;
	float	m_flAccuracy;
	float	m_flLastFire;
	int		m_iShotsFired;
	float	m_vVecAiming_x;
	float	m_vVecAiming_y;
	float	m_vVecAiming_z;
	uint	model_name;
	float	m_flGlock18Shoot;
	int		m_iGlock18ShotsFired;
	float	m_flFamasShoot;
	int		m_iFamasShotsFired;
	float	m_fBurstSpread;
	int		m_iWeaponState;
	float	m_flNextReload;
	float	m_flDecreaseShotsFired;
	uint16	m_usFireGlock18;
	uint16	m_usFireFamas;
	float	m_flPrevPrimaryAttack;
	float	m_flLastFireTime;
	bool	m_bStartedArming;
	bool	m_bBombPlacedAnimation;
	char	pad_0x0142[0x2];
	float	m_fArmedTime;
	char	pad_0x0148[0x8];
};

class CBaseWeapon
{
public:
	void AllocateMemory();
	void FreeMemory();

	void Update(local_state_s* from, local_state_s* to, usercmd_s* cmd, double time, unsigned random_seed);

	bool CanAttack();
	bool IsGun();
	bool IsKnife();
	bool IsNonAttack();
	bool IsNade();
	bool IsC4();
	bool IsSniper();
	bool IsPistol();
	bool IsRifle();
	bool IsShotGun();
	bool IsMachineGun();
	bool IsSubMachineGun();
	bool IsSilenced();
	bool IsInBurst();
	bool IsLeftElite();

	int GetClassType();
	int GetDamage();
	float GetWallPierce();

	CBaseWpnInfo* operator->();

private:
	std::unique_ptr<CBaseWpnInfo> pBaseInfo;

	CCSPlayerWeapon* GetCSWeaponData(int iWeaponID = WEAPON_NONE);

	int GetWeaponAccuracyFlags();

	void UpdateShotsFired(usercmd_s* cmd);
};

extern CBaseWeapon g_Weapon;