class CMiscellaneous;
class CMiscellaneous
{
private:
	bool		_bFakeLagActive;

	float		_flFadeEnd;

public:
	bool		m_bFakeWalkActive;
	bool		m_bFakeLatencyActive;
	bool		m_bPositionAdjustmentActive;
	short		m_sPositionAdjustmentLerp;
	short		m_sPositionAdjustmentLerpVisual;

	float		m_flFlashed;
	
	Vector		m_vecRealAngles;

	unsigned short	hud_buttons;

	void	Brightness();
	void	FakeLatency();
	void	NameStealer();
	void	AutoPistol(usercmd_s *cmd);
	void	AutoReload(usercmd_s *cmd);
	void	AntiAim(usercmd_s *cmd);
	void	FakeLag(usercmd_s *cmd);
	void	FakeWalk(usercmd_s *cmd);
	void	ThirdPerson(ref_params_s *pparams);
	void	ScreenFade();
	void	HUDCommands(usercmd_s *cmd);
	void	CustomRenderFov(const int value);
	void	MaximizeWindowAfterRespawn();
};
extern CMiscellaneous g_Miscellaneous;
