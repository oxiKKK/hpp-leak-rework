class Physent
{
public:
	static void SetOrigin(const int& index, const Vector& origin);
	static void SetSolid(const int& index, const int& solid);
	static void SetMaxs(const int& index, const Vector& maxs);
	static void SetMins(const int& index, const Vector& mins);
	static void SetAngles(const int& index, const QAngle& angles);

private:
	static physent_t* GetPhysent(const int& index);
};