class CDefaultCrosshair
{
public:
	void Draw();

private:
	float	m_flCrosshairDistance;
	int		m_iCrosshairScaleBase;
	int		m_iAmmoLastCheck;
	int		m_iColors[3];

	void DrawCrosshairEx(int iBarSize, float flCrosshairDistance);

	void CalcCrosshairColor();
	void CalcCrosshairSize();
};