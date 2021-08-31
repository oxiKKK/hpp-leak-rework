class CMiscellaneous
{
public:
	CMiscellaneous();
	~CMiscellaneous();

	int				m_iChokedCommands;
	int				m_iHudCommands;

	bool			m_bFakeLatencyActive;
	bool			m_bPositionAdjustmentActive;

	float			m_flPositionAdjustmentInterpAmount;
	float			GetInterpAmount(const int &lerp);

	void			ChokedCommandsCounter();
	void			AutoReload(usercmd_s* cmd);
	void			AutoPistol(usercmd_s* cmd);
	void			NameStealer();
	void			FakeLatency();
	void			RecordHUDCommands(usercmd_s* cmd);

};

extern std::unique_ptr<CMiscellaneous> g_pMiscellaneous;