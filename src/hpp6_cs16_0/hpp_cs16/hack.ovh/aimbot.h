class CAimbot;
class CAimbot {
private:
	bool m_bTriggerActive;
	bool m_bAimbotActive;

	void RageBot(usercmd_s* cmd);
	void LegitBot(usercmd_s* cmd);
	void SmoothAimAngles(QAngle MyViewAngles, QAngle AimAngles, QAngle &OutAngles, float Smoothing);
public:
	bool m_bTriggerStatus;
	float m_flLegitBotFOV = 0;
	DWORD m_flLastTimeKilled;

	void Trigger(usercmd_s* cmd);
	void Run(usercmd_s* cmd);
};

extern CAimbot g_Aimbot;

class CTarget;
class CTarget {
public:
	bool	m_bValid;
	int		m_iScore;
	int		m_iHitbox;
	int		m_iPoint;
	CBasePlayer *m_pPlayer;

	CTarget();
};