class CLegitBot
{
public:
	CLegitBot();
	~CLegitBot();

	bool					m_bAimState;
	bool					m_bAimPerfectSilentState;
	bool					m_bTriggerState;

	int						m_iAimPlayer;
	int						m_iAimHitbox;

	float					m_flCurrentFOV;

	void					Run(usercmd_s* cmd);

private:
	float					m_flMinAngleDemoChecker;

	void					Aimbot(usercmd_s* cmd);
	void					Trigger(usercmd_s* cmd);
	void					StandaloneRecoilControl(usercmd_s* cmd);
	void					CorrectPhysentSolid(const int& nPlayerID);
	void					SmoothAimAngles(const QAngle& QAngles, const QAngle& QAimAngles, QAngle& QNewAngles, const float& flSmoothing);
	void					DesyncHelper(usercmd_s* cmd);

	bool					DemoChecker(const QAngle& a_QPreviousAngles, const QAngle& a_QNewAngles, QAngle& a_QCorrectedAngles);
};

extern std::unique_ptr<CLegitBot> g_pLegitBot;