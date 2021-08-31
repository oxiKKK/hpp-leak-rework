class CMisc;
class CMisc
{
private:
	bool m_bFakeLagActive = false;

	bool Edge(float &angle);
public:
	bool m_bFakeLatencyActive = false;
	Vector m_vRealAngles = {};

	void AddFakeLatency(float flLatency);
	void AntiAim(struct usercmd_s *cmd);
	void ThirdPerson(struct ref_params_s *pparams);
	void FakeLag(struct usercmd_s *cmd);
	void AutoPistol(struct usercmd_s *cmd);
	void AutoReload(struct usercmd_s *cmd);

	void SimulatePlayerMove(playermove_t *playermove);
};
extern CMisc g_Misc;
