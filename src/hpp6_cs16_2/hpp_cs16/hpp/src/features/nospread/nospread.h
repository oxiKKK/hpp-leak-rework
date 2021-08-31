enum ENoSpread
{
	NOSPREAD_NONE,
	NOSPREAD_PITCH_YAW,
	NOSPREAD_PITCH_ROLL,
	NOSPREAD_PITCH_YAW_ROLL
};

class CNoSpread
{
public:
	void GetSpreadXY(const unsigned& random_seed, const int& future, Vector& vecSpread);
	void GetSpreadOffset(const unsigned& random_seed, const int& future, const QAngle& QInAngles, QAngle& QOutAngles, const int& type = NOSPREAD_PITCH_YAW_ROLL);
	
private:
	double GetConstantRemainder(const Vector& vecSpread);
};

extern std::unique_ptr<CNoSpread> g_pNoSpread;