class CNoSpread;
class CNoSpread {
private:
	double GetConstantRemainder(Vector Spread);
public:
	void GetSpreadXY(unsigned int a_iRandomSeed, int a_iFuture, Vector &vecSpread, bool a_bSimulate = false);
	void GetSpreadOffset(unsigned int a_iRandomSeed, int a_iFuture, QAngle a_QInAngles, QAngle &a_QOutAngles, int a_nType = 2);
};
extern CNoSpread g_NoSpread;