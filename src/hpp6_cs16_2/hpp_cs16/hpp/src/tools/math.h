class Math
{
public:
	static void		VectorTransform(Vector vecInput, matrix3x4_t matrix, const Vector& vecOutput);
	static void		VectorTransform(Vector vecInput, matrix3x4_t matrix, const Vector& vecOutput, const int &xyz, const float &multi);
	static void		VectorIRotate(const Vector& vecInput, const matrix3x4_t& in2, Vector& vecOutput);
	static void		VectorITransform(const Vector& vecInput, const matrix3x4_t& in2, Vector& vecOutput);

	static void		ComputeMove(Vector vecVelocity, const QAngle& QAngles, float* forwardmove, float* sidemove);

	static bool		IntersectRayWithOBB(const Vector& vecRayStart, const Vector& vecRayDelta, const matrix3x4_t& matOBBToWorld, const Vector& vecOBBMins, const Vector& vecOBBMaxs, float& fraction, int& hitside, bool& startsolid);
	static bool		IntersectRayWithBox(const Vector& rayStart, const Vector& rayDelta, const Vector& boxMins, const Vector& boxMaxs, float& fraction, int& hitside, bool& startsolid);

	static void		NormalizeAngles(const QAngle& QAngles);
	static void		VectorAngles(const Vector& vecForward, QAngle& QAngles);

	static double	Interp(double s1, double s2, double s3, double f1, double f3);
	static double	Interp_F2(double s1, double s2, double s3, double f1, double f3);
	static double	Interp_S2(double s1, double s3, double f1, double f2, double f3);
	static float	TranslateToEvenGreatest(float val);
};