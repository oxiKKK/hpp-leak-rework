class CNoSpread;
class CNoSpread {
private:
	double _GetConstantRemainder(Vector a_vecSpread);

public:
	void Run(usercmd_s *cmd);
	void GetSpreadXY(unsigned int a_iRandomSeed, int a_iFuture, Vector &vecSpread);
	void GetSpreadOffset(unsigned int a_iRandomSeed, int a_iFuture, QAngle a_QInAngles, QAngle &a_QOutAngles, int a_nType = 2);
};
extern CNoSpread g_NoSpread;