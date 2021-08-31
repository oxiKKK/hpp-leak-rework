class CAimbot;
class CAimbot {
private:
	bool		_bTriggerActive;
	bool		_bAimbotActive;

	void		_Trigger(usercmd_s* cmd);
	void		_RageBot(usercmd_s* cmd);
	void		_LegitBot(usercmd_s* cmd);
	void		_SmoothAimAngles(const QAngle a_QAngles, const QAngle a_QAimAngles, QAngle &a_QNewAngles, const float a_flSmoothing);

public:
	float		m_flLegitBotFOV;
	double		m_dbLastTimeKilled;

	void		Run(usercmd_s* cmd);
};
extern CAimbot g_Aimbot;

class CTarget;
class CTarget {
public:
	int		m_iScore;
	int		m_iHitbox;
	int		m_iPoint;
	Vector	m_vecCorrectedOrigin;
	CBasePlayer *m_pPlayer;

	CTarget();
};