enum Penerations
{
	WALL_PEN0,
	WALL_PEN1,
	WALL_PEN2,
};


class CCSPlayerWeapon
{
public:
	char pad_0x0000[0x78]; //0x0000
	float m_flStartThrow; //0x0078 
	float m_flReleaseThrow; //0x007C 
	__int32 m_iSwing; //0x0080 
	char pad_0x0084[0x28]; //0x0084
	__int32 m_iId; //0x00AC 
	__int32 m_iPlayEmptySound; //0x00B0 
	__int32 m_fFireOnEmpty; //0x00B4 
	float m_flNextPrimaryAttack; //0x00B8 
	float m_flNextSecondaryAttack; //0x00BC 
	float m_flTimeWeaponIdle; //0x00C0 
	__int32 m_iPrimaryAmmoType; //0x00C4 
	__int32 m_iSecondaryAmmoType; //0x00C8 
	__int32 m_iClip; //0x00CC 
	__int32 m_iClientClip; //0x00D0 
	__int32 m_iClientWeaponState; //0x00D4 
	__int32 m_fInReload; //0x00D8 
	__int32 m_fInSpecialReload; //0x00DC 
	__int32 m_iDefaultAmmo; //0x00E0 
	__int32 m_iShellId; //0x00E4 
	float m_fMaxSpeed; //0x00E8 
	__int32 m_bDelayFire; //0x00EC 
	__int32 m_iDirection; //0x00F0 
	__int32 m_bSecondarySilencerOn; //0x00F4 
	float m_flAccuracy; //0x00F8 
	float m_flLastFire; //0x00FC 
	__int32 m_iShotsFired; //0x0100 
	float m_vVecAiming_x; //0x0104 
	float m_vVecAiming_y; //0x0108 
	float m_vVecAiming_z; //0x010C 
	__int32 model_name; //0x0110 
	float m_flGlock18Shoot; //0x0114 
	__int32 m_iGlock18ShotsFired; //0x0118 
	float m_flFamasShoot; //0x011C 
	__int32 m_iFamasShotsFired; //0x0120 
	float m_fBurstSpread; //0x0124 
	__int32 m_iWeaponState; //0x0128 
	float m_flNextReload; //0x012C 
	float m_flDecreaseShotsFired; //0x0130 
	__int16 m_usFireGlock18; //0x0134 
	__int16 m_usFireFamas; //0x0136 
	float m_flPrevPrimaryAttack; //0x0138 
	float m_flLastFireTime; //0x013C 
	unsigned char m_bStartedArming; //0x0140 
	unsigned char m_bBombPlacedAnimation; //0x0141 
	char pad_0x0142[0x2]; //0x0142
	float m_fArmedTime; //0x0144 m_flPumpTime
	char pad_0x0148[0x8]; //0x0148
}; //Size=0x0150

class CBaseWeaponData
{
public:
	CBaseWeaponData()	{ RtlSecureZeroMemory(this, sizeof(*this)); }

	unsigned int		m_iRandomSeed;

	int					m_iWeaponState;
	int					m_iFlags;
	int					m_iClip;
	int					m_iWeaponID;
	int					m_iInReload;
	int					m_iPenetration;
	int					m_iBulletType;
	int					m_iDamage1;
	int					m_iDamage2;
	int					m_iUser3;
	int					m_iShotsFired;

	float				m_flAccuracy;
	float				m_flSpread;
	float				m_flNextPrimaryAttack;
	float				m_flNextSecondaryAttack;
	float				m_flNextAttack;
	float				m_flTimeWeaponIdle;
	float				m_flStartThrow;
	float				m_flReleaseThrow;
	float				m_flPenetrationDistance;
	float				m_flWallPierce1;
	float				m_flWallPierce2;
};

class CBasePlayerWeapon
{
private:
	void				_UpdateWeaponData();

public:
	CBaseWeaponData		data;

	void				ItemPreFrame(local_state_s* to, usercmd_s* cmd, int runfuncs, double time, unsigned int random_seed);
	void				Update();
	CCSPlayerWeapon		GetCSWeapon(int iWeaponID);

	bool				CanAttack(void);
	bool				IsGun(void);
	bool				IsKnife(void);
	bool				IsNonAttack(void);
	bool				IsNade(void);
	bool				IsC4(void);
	bool				IsSniper(void);
	bool				IsPistol(void);
	bool				IsRifle(void);
	bool				IsShotGun(void);
	bool				IsMachineGun(void);
	bool				IsSubMachineGun(void);
	bool				IsSilenced(void);
	bool				IsInBurst(void);
	bool				IsLeftElite(void);
	bool				IsFreezePeriod(void);

	int					GetClassType(void);
	int					GetPenetration(void);
	int					GetBulletType(void);
	int					GetDamage(void);

	float				GetWallPierce(void);
	float				GetDistance(void);
};
extern CBasePlayerWeapon g_Weapon;