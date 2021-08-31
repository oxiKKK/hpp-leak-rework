class QAngle;
class Vector;

class Vector {
public:
	Vector(void)
		: x(0)
		, y(0)
		, z(0)
	{
	}

	Vector(float X, float Y, float Z)
		: x(X)
		, y(Y)
		, z(Z)
	{
	}

	Vector(float* p)
	{
		*this = p;
	};

	Vector(const Vector& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;
	};

	~Vector()
	{
	}

	inline Vector& operator=(const Vector& in);
	inline Vector& operator=(float* p);
	inline Vector& operator=(float f);

	inline float& operator[](int i) const;

	inline bool operator!(void)const;

	inline bool operator==(const Vector& other) const;

	inline bool operator!=(const Vector& other) const;

	inline Vector& operator+=(const Vector& other);
	inline Vector& operator+=(float* p);
	inline Vector& operator+=(float f);

	inline Vector& operator-=(const Vector& other);
	inline Vector& operator-=(float* p);
	inline Vector& operator-=(float f);

	inline Vector& operator*=(const Vector& other);
	inline Vector& operator*=(float* p);
	inline Vector& operator*=(float f);

	inline Vector& operator/=(const Vector& other);
	inline Vector& operator/=(float* p);
	inline Vector& operator/=(float f);

	inline Vector operator+(const Vector& other) const;
	inline Vector operator+(float* p) const;
	inline Vector operator+(float f) const;

	inline Vector operator-(const Vector& other) const;
	inline Vector operator-(float* p) const;
	inline Vector operator-(float f) const;
	inline Vector operator-(void) const;

	inline Vector operator*(const Vector& other) const;
	inline Vector operator*(float* p) const;
	inline Vector operator*(float f) const;

	inline Vector operator/(const Vector& other) const;
	inline Vector operator/(float* p) const;
	inline Vector operator/(float f) const;

	operator float*() { return &x; }
	operator const float*() const { return &x; }

	inline bool IsZero(void) const
	{
		return x == 0 && y == 0 && z == 0;
	}

	inline bool IsZero2D(void)
	{
		return x == 0 && y == 0;
	}

	inline Vector& Clear(void)
	{
		x = y = z = 0;

		return *this;
	}

	inline Vector& Init(float X, float Y, float Z)
	{
		x = X;
		y = Y;
		z = Z;

		return *this;
	}

	inline Vector& Init(float* p)
	{
		*this = p;

		return *this;
	}

	inline Vector& Negate(void)
	{
		x = -x;
		y = -y;
		z = -z;

		return *this;
	}

	inline float Dot(const Vector& other)
	{
		return x * other.x + y * other.y + z * other.z;
	}

	inline float Dot2D(const Vector& other)
	{
		return x * other.x + y * other.y;
	}

	inline float Length(void)
	{
		return sqrtf(LengthSqr());
	}

	inline float Length2D(void)
	{
		return sqrtf(LengthSqr2D());
	}

	inline float LengthSqr(void)
	{
		return x * x + y * y + z * z;
	}

	inline float LengthSqr2D(void)
	{
		return x * x + y * y;
	}

	inline float Distance(Vector& ToVector)
	{
		return (ToVector - *this).Length();
	}

	inline float Distance2D(Vector& ToVector)
	{
		return (ToVector - *this).Length2D();
	}

	inline float Normalize(void)
	{
		float VectorLength = Length();

		if (VectorLength) {
			float RecipLength = 1 / VectorLength;

			x *= RecipLength;
			y *= RecipLength;
			z *= RecipLength;
		}

		return VectorLength;
	}

	inline float Normalize2D(void)
	{
		float Length = LengthSqr2D();

		if (Length) {
			float RecipLength = 1 / Length;

			x *= RecipLength;
			y *= RecipLength;
		}

		return Length;
	}

	inline float AngleBetween(Vector& other)
	{
		other.Normalize();
		Normalize();

		return RAD2DEG(acosf(Dot(other)));
	}

	Vector& CrossProduct(const Vector& a, const Vector& b)
	{
		x = (a.y * b.z) - (a.z * b.y);
		y = (a.z * b.x) - (a.x * b.z);
		z = (a.x * b.y) - (a.y * b.x);

		return *this;
	}

	QAngle ToEulerAngles();
	QAngle ToEulerAngles(Vector* PseudoUp);
	void AngleMatrix(QAngle& Rotation, float(*matrix)[3]);
	void VectorRotate(Vector& In, QAngle& Rotation);

	void VectorVectors(Vector& Right, Vector& Up)
	{
		if (!x && !y) {
			Right[0] = 1;
			Right[1] = Right[2] = 0;
			Up[0] = -z;
			Up[1] = Up[2] = 0;
			return;
		}

		Vector tmp;
		tmp[0] = tmp[1] = 0;
		tmp[2] = 1;

		CrossProduct(tmp, Right);

		Right.Normalize();
		Right.CrossProduct(*this, Up);
		Up.Normalize();
	}

	inline bool IsValid()
	{
		return isfinite(x) && isfinite(y) && isfinite(z);
	}

public:
	float x, y, z;
};

inline Vector& Vector::operator=(const Vector& in)
{
	x = in.x;
	y = in.y;
	z = in.z;

	return *this;
}

inline Vector& Vector::operator=(float* p)
{
	if (p) {
		x = p[0];
		y = p[1];
		z = p[2];
	}
	else {
		x = y = z = 0;
	}

	return *this;
}

inline Vector& Vector::operator=(float f)
{
	x = y = z = f;

	return *this;
}

inline float& Vector::operator[](int i) const
{
	if (i >= 0 && i < 3)
		return ((float*)this)[i];

	return ((float*)this)[0];
}

inline bool Vector::operator!(void)const
{
	return IsZero();
}

inline bool Vector::operator==(const Vector& other) const
{
	return x == other.x && y == other.y && z == other.z;
}

inline bool Vector::operator!=(const Vector& other) const
{
	return x != other.x || y != other.y || z != other.z;
}

inline Vector& Vector::operator+=(const Vector& other)
{
	x += other.x;
	y += other.y;
	z += other.z;

	return *this;
}

inline Vector& Vector::operator+=(float* p)
{
	x += p[0];
	y += p[1];
	z += p[2];

	return *this;
}

inline Vector& Vector::operator+=(float f)
{
	x += f;
	y += f;
	z += f;

	return *this;
}

inline Vector& Vector::operator-=(const Vector& other)
{
	x -= other.x;
	y -= other.y;
	z -= other.z;

	return *this;
}

inline Vector& Vector::operator-=(float* p)
{
	x -= p[0];
	y -= p[1];
	z -= p[2];

	return *this;
}
inline Vector& Vector::operator-=(float f)
{
	x -= f;
	y -= f;
	z -= f;

	return *this;
}

inline Vector& Vector::operator*=(const Vector& other)
{
	x *= other.x;
	y *= other.y;
	z *= other.z;

	return *this;
}

inline Vector& Vector::operator*=(float* p)
{
	x *= p[0];
	y *= p[1];
	z *= p[2];

	return *this;
}

inline Vector& Vector::operator*=(float f)
{
	x *= f;
	y *= f;
	z *= f;

	return *this;
}

inline Vector& Vector::operator/=(const Vector& other)
{
	if (other.x && other.y && other.z) {
		x /= other.x;
		y /= other.y;
		z /= other.z;
	}

	return *this;
}

inline Vector& Vector::operator/=(float* p)
{
	if (p[0] && p[1] && p[2]) {
		x /= p[0];
		y /= p[1];
		z /= p[2];
	}

	return *this;
}

inline Vector& Vector::operator/=(float f)
{
	if (f) {
		x /= f;
		y /= f;
		z /= f;
	}

	return *this;
}

inline Vector Vector::operator+(const Vector& other) const
{
	return Vector(x + other.x, y + other.y, z + other.z);
}

inline Vector Vector::operator+(float* p) const
{
	return Vector(x + p[0], y + p[1], z + p[2]);
}

inline Vector Vector::operator+(float f) const
{
	return Vector(x + f, y + f, z + f);
}

inline Vector Vector::operator-(const Vector& other) const
{
	return Vector(x - other.x, y - other.y, z - other.z);
}

inline Vector Vector::operator-(float* p) const
{
	return Vector(x - p[0], y - p[1], z - p[2]);
}

inline Vector Vector::operator-(float f) const
{
	return Vector(x - f, y - f, z - f);
}

inline Vector Vector::operator-(void) const
{
	return Vector(-x, -y, -z);
}

inline Vector Vector::operator*(const Vector& other) const
{
	return Vector(x * other.x, y * other.y, z * other.z);
}

inline Vector Vector::operator*(float* p) const
{
	return Vector(x * p[0], y * p[1], z * p[2]);
}

inline Vector Vector::operator*(float f) const
{
	return Vector(x * f, y * f, z * f);
}

inline Vector Vector::operator/(const Vector& other) const
{
	if (other.x && other.y && other.z)
		return Vector(x / other.x, y / other.y, z / other.z);

	return *this;
}

inline Vector Vector::operator/(float* p) const
{
	if (p[0] && p[1] && p[2])
		return Vector(x / p[0], y / p[1], z / p[2]);

	return *this;
}

inline Vector Vector::operator/(float f) const
{
	if (f)
		return Vector(x / f, y / f, z / f);

	return *this;
}

inline Vector operator*(float fl, const Vector& v) { return v * fl; };

#define VectorAdd(a, b, c) \
    {                      \
        a.x = b.x + c.x;   \
        a.y = b.y + c.y;   \
        a.z = b.z + c.z;   \
    } // быстрее, чем перегрузка +, но на эту можна забить
#define VectorCopy(a, b) \
    {                    \
        a.x = b.x;       \
        a.y = b.y;       \
        a.z = b.z;       \
    } // быстрее, чем перегрузка =