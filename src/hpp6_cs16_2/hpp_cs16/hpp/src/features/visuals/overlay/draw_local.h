struct ToggleStatusData
{
	std::string name;
	bool status;
	double timestamp;
};

class CDrawLocal
{
public:
	CDrawLocal();

	void Overlay();

private:
	std::unique_ptr<CDefaultCrosshair> m_pDefaultCrosshair;

	void SniperCrosshair();
	void AimFOV();
	void RecoilPoint();
	void SpreadPoint();
	void SpreadCircle();
	void ToggleStatus();
};

extern ToggleStatusData g_ToggleStatus;