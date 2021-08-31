#pragma once

class QAngle;

class Vector2D
{
public:
	float x, y;

	inline Vector2D() :
		x(0),
		y(0)
	{
	}

	inline Vector2D(float _x, float _y) :
		x(_x),
		y(_y)
	{
	}

	inline Vector2D(float v) :
		x(v),
		y(v)
	{
	}

	inline Vector2D(float* p)
	{
		*this = p;
	};

	inline Vector2D(const Vector2D& in) :
		x(in.x),
		y(in.y)
	{
	};

	inline ~Vector2D()
	{
	}

	inline bool IsZero() const
	{
		return x == 0 && y == 0;
	}

	inline Vector2D& Clear()
	{
		x = 0.f;
		y = 0.f;

		return *this;
	}

	inline Vector2D& Init(float _x, float _y)
	{
		x = _x;
		y = _y;

		return *this;
	}

	inline Vector2D& Init(float* p)
	{
		*this = p;

		return *this;
	}

	inline Vector2D& Negate()
	{
		x = -x;
		y = -y;

		return *this;
	}

	inline float Dot(const Vector2D& other)
	{
		return x * other.x + y * other.y;
	}

	inline float LengthSqr()
	{
		return x * x + y * y;
	}

	inline float Length()
	{
		return sqrt(LengthSqr());
	}

	inline float Distance(const Vector2D& ToVector)
	{
		return (ToVector - *this).Length();
	}

	inline Vector2D GetNormalize()
	{
		const float flVectorLength = Length();
		float flRecipLength = 1.f;

		if (flVectorLength)
			flRecipLength /= flVectorLength;

		return Vector2D(x * flRecipLength, y * flRecipLength);
	}

	inline float Normalize()
	{
		float flVectorLength = Length();

		if (flVectorLength)
		{
			float flRecipLength = 1.f / flVectorLength;

			x *= flRecipLength;
			y *= flRecipLength;
		}

		return flVectorLength;
	}

	inline float AngleBetween(Vector2D other)
	{
		other.Normalize();
		Normalize();

		auto angle = RAD2DEG(acos(Dot(other)));

		if (isfinite(angle))
			return angle;

		return 0.f;
	}

	inline Vector2D& operator=(const Vector2D& in)
	{
		x = in.x;
		y = in.y;

		return *this;
	}

	inline Vector2D& operator=(float* p)
	{
		if (p)
		{
			x = p[0];
			y = p[1];
		}
		else
		{
			x = 0.f;
			y = 0.f;
		}

		return *this;
	}

	inline Vector2D& operator=(float v)
	{
		x = v;
		y = v;

		return *this;
	}

	inline float& operator[](int i) const
	{
		if (i >= 0 && i < 23)
			return ((float*)this)[i];

		return ((float*)this)[0];
	}

	inline bool operator!() const
	{
		return IsZero();
	}

	inline bool operator==(const Vector2D& other) const
	{
		return x == other.x && y == other.y;
	}

	inline bool operator!=(const Vector2D& other) const
	{
		return x != other.x || y != other.y;
	}

	inline Vector2D& operator+=(const Vector2D& other)
	{
		x += other.x;
		y += other.y;

		return *this;
	}

	inline Vector2D& operator+=(float* p)
	{
		x += p[0];
		y += p[1];

		return *this;
	}

	inline Vector2D& operator+=(float v)
	{
		x += v;
		y += v;

		return *this;
	}

	inline Vector2D& operator-=(const Vector2D& other)
	{
		x -= other.x;
		y -= other.y;

		return *this;
	}

	inline Vector2D& operator-=(float* p)
	{
		x -= p[0];
		y -= p[1];

		return *this;
	}

	inline Vector2D& operator-=(float v)
	{
		x -= v;
		y -= v;

		return *this;
	}

	inline Vector2D& operator*=(const Vector2D& other)
	{
		x *= other.x;
		y *= other.y;

		return *this;
	}

	inline Vector2D& operator*=(float* p)
	{
		x *= p[0];
		y *= p[1];

		return *this;
	}

	inline Vector2D& operator*=(float v)
	{
		x *= v;
		y *= v;

		return *this;
	}

	inline Vector2D& operator/=(const Vector2D& other)
	{
		x /= other.x;
		y /= other.y;

		return *this;
	}

	inline Vector2D& operator/=(float* p)
	{
		x /= p[0];
		y /= p[1];

		return *this;
	}

	inline Vector2D& operator/=(float v)
	{
		x /= v;
		y /= v;

		return *this;
	}

	inline Vector2D operator+(const Vector2D& other) const
	{
		return Vector2D(x + other.x, y + other.y);
	}

	inline Vector2D operator+(float* p) const
	{
		return Vector2D(x + p[0], y + p[1]);
	}

	inline Vector2D operator+(float v) const
	{
		return Vector2D(x + v, y + v);
	}

	inline Vector2D operator-(const Vector2D& other) const
	{
		return Vector2D(x - other.x, y - other.y);
	}

	inline Vector2D operator-(float* p) const
	{
		return Vector2D(x - p[0], y - p[1]);
	}

	inline Vector2D operator-(float v) const
	{
		return Vector2D(x - v, y - v);
	}

	inline Vector2D operator-() const
	{
		return Vector2D(-x, -y);
	}

	inline Vector2D operator*(const Vector2D& other) const
	{
		return Vector2D(x * other.x, y * other.y);
	}

	inline Vector2D operator*(float* p) const
	{
		return Vector2D(x * p[0], y * p[1]);
	}

	inline Vector2D operator*(float v) const
	{
		return Vector2D(x * v, y * v);
	}

	inline Vector2D operator/(const Vector2D& other) const
	{
		return Vector2D(x / other.x, y / other.y);
	}

	inline Vector2D operator/(float* p) const
	{
		return Vector2D(x / p[0], y / p[1]);
	}

	inline Vector2D operator/(float v) const
	{
		return Vector2D(x / v, y / v);
	}

	operator float* ()
	{
		return &x;
	}

	operator const float* () const
	{
		return &x;
	}
};

class Vector
{
public:
	float x, y, z;

	inline Vector() :
		x(0), 
		y(0), 
		z(0)
	{
	}

	inline Vector(float _x, float _y, float _z) :
		x(_x), 
		y(_y), 
		z(_z)
	{
	}

	inline Vector(float v) :
		x(v),
		y(v),
		z(v)
	{
	}

	inline Vector(float* p)
	{
		*this = p;
	};

	inline Vector(const Vector& in) :
		x(in.x),
		y(in.y),
		z(in.z)
	{
	};

	inline ~Vector()
	{
	}

	inline Vector2D ToVec2D()
	{
		return Vector2D(x, y);
	}

	inline bool IsZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}

	inline bool IsZero2D() const
	{
		return x == 0 && y == 0;
	}

	inline Vector& Clear()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;

		return *this;
	}

	inline Vector& Init(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;

		return *this;
	}

	inline Vector& Init(float* p)
	{
		*this = p;

		return *this;
	}

	inline Vector& Negate()
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

	inline float LengthSqr()
	{
		return x * x + y * y + z * z;
	}

	inline float LengthSqr2D(void)
	{
		return x * x + y * y;
	}

	inline float Length()
	{
		return sqrtf(LengthSqr());
	}

	inline float Length2D(void)
	{
		return sqrtf(LengthSqr2D());
	}

	inline float Distance(const Vector& ToVector)
	{
		return (ToVector - *this).Length();
	}

	inline Vector GetNormalize()
	{
		const float flVectorLength = Length();
		float flRecipLength = 1.f;

		if (flVectorLength)
			flRecipLength /= flVectorLength;

		return Vector(x * flRecipLength, y * flRecipLength, z * flRecipLength);
	}

	inline float Normalize()
	{
		float flVectorLength = Length();

		if (flVectorLength)
		{
			float flRecipLength = 1.f / flVectorLength;

			x *= flRecipLength;
			y *= flRecipLength;
			z *= flRecipLength;
		}

		return flVectorLength;
	}

	inline float AngleBetween(Vector other)
	{
		other.Normalize();
		Normalize();

		auto angle = RAD2DEG(acos(Dot(other)));

		if (isfinite(angle))
			return angle;

		return 0.f;
	}

	Vector& CrossProduct(const Vector& a, const Vector& b)
	{
		assert(a != *this);
		assert(b != *this);

		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;

		return *this;
	}

	inline Vector& operator=(const Vector& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;

		return *this;
	}

	inline Vector& operator=(float* p)
	{
		if (p)
		{
			x = p[0];
			y = p[1];
			z = p[2];
		}
		else
		{
			x = 0.f;
			y = 0.f;
			z = 0.f;
		}

		return *this;
	}

	inline Vector& operator=(const float* p)
	{
		if (p)
		{
			x = p[0];
			y = p[1];
			z = p[2];
		}
		else
		{
			x = 0.f;
			y = 0.f;
			z = 0.f;
		}

		return *this;
	}

	inline Vector& operator=(float v)
	{
		x = v;
		y = v;
		z = v;

		return *this;
	}

	inline float& operator[](int i) const
	{
		if (i >= 0 && i < 3)
			return ((float*)this)[i];

		return ((float*)this)[0];
	}

	inline bool operator!() const
	{
		return IsZero();
	}

	inline bool operator==(const Vector& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	inline bool operator!=(const Vector& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	inline Vector& operator+=(const Vector& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	inline Vector& operator+=(float* p)
	{
		x += p[0];
		y += p[1];
		z += p[2];

		return *this;
	}

	inline Vector& operator+=(float v)
	{
		x += v;
		y += v;
		z += v;

		return *this;
	}

	inline Vector& operator-=(const Vector& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	inline Vector& operator-=(float* p)
	{
		x -= p[0];
		y -= p[1];
		z -= p[2];

		return *this;
	}

	inline Vector& operator-=(float v)
	{
		x -= v;
		y -= v;
		z -= v;

		return *this;
	}

	inline Vector& operator*=(const Vector& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;

		return *this;
	}

	inline Vector& operator*=(float* p)
	{
		x *= p[0];
		y *= p[1];
		z *= p[2];

		return *this;
	}

	inline Vector& operator*=(float v)
	{
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}

	inline Vector& operator/=(const Vector& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;

		return *this;
	}

	inline Vector& operator/=(float* p)
	{
		x /= p[0];
		y /= p[1];
		z /= p[2];

		return *this;
	}

	inline Vector& operator/=(float v)
	{
		x /= v;
		y /= v;
		z /= v;

		return *this;
	}

	inline Vector operator+(const Vector& other) const
	{
		return Vector(x + other.x, y + other.y, z + other.z);
	}

	inline Vector operator+(float* p) const
	{
		return Vector(x + p[0], y + p[1], z + p[2]);
	}

	inline Vector operator+(float v) const
	{
		return Vector(x + v, y + v, z + v);
	}

	inline Vector operator-(const Vector& other) const
	{
		return Vector(x - other.x, y - other.y, z - other.z);
	}

	inline Vector operator-(float* p) const
	{
		return Vector(x - p[0], y - p[1], z - p[2]);
	}

	inline Vector operator-(float v) const
	{
		return Vector(x - v, y - v, z - v);
	}

	inline Vector operator-() const
	{
		return Vector(-x, -y, -z);
	}

	inline Vector operator*(const Vector& other) const
	{
		return Vector(x * other.x, y * other.y, z * other.z);
	}

	inline Vector operator*(float* p) const
	{
		return Vector(x * p[0], y * p[1], z * p[2]);
	}

	inline Vector operator*(float v) const
	{
		return Vector(x * v, y * v, z * v);
	}
	
	inline Vector operator/(const Vector& other) const
	{
		return Vector(x / other.x, y / other.y, z / other.z);
	}

	inline Vector operator/(float* p) const
	{
		return Vector(x / p[0], y / p[1], z / p[2]);
	}

	inline Vector operator/(float v) const
	{
		return Vector(x / v, y / v, z / v);
	}

	operator float* () 
	{ 
		return &x; 
	}

	operator const float* () const 
	{ 
		return &x; 
	}

	void VectorVectors(Vector& Right, Vector& Up)
	{
		if (ToVec2D().IsZero()) 
		{
			Right.Init(1.f, 0.f, 0.f);
			Up.Init(-z, 0.f, 0.f);
			return;
		}

		Vector tmp(0.f, 0.f, 1.f);
		CrossProduct(tmp, Right);

		Right.Normalize();
		Right.CrossProduct(*this, Up);
		Up.Normalize();
	}

	//QAngle ToEulerAngles();
	QAngle ToEulerAngles(Vector* PseudoUp);
	void AngleMatrix(const QAngle& Rotation, float(*matrix)[3]);
	void VectorRotate(Vector& In, const QAngle& Rotation);
};

class Vector4D
{
public:
	float x, y, z, w;

	inline Vector4D() :
		x(0), 
		y(0), 
		z(0), 
		w(0)
	{
	}

	inline Vector4D(float _x, float _y, float _z, float _w) :
		x(_x),
		y(_y),
		z(_z),
		w(_w)
	{
	}

	inline Vector4D(float v) :
		x(v),
		y(v),
		z(v),
		w(v)
	{
	}

	inline Vector4D(float* p)
	{
		*this = p;
	};

	inline Vector4D(const Vector4D& in) :
		x(in.x),
		y(in.y),
		z(in.z),
		w(in.w)
	{
	};

	inline ~Vector4D()
	{
	}

	inline Vector ToVec3D()
	{
		return Vector(x, y, z);
	}

	inline Vector2D ToVec2D()
	{
		return Vector2D(x, y);
	}

	inline bool IsZero() const
	{
		return x == 0 && y == 0 && z == 0 && w == 0;
	}

	inline Vector4D& Clear()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;
		w = 0.f;

		return *this;
	}

	inline Vector4D& Init(float _x, float _y, float _z, float _w)
	{
		x = _x;
		y = _y;
		z = _z;
		w = _w;

		return *this;
	}

	inline Vector4D& Init(float* p)
	{
		*this = p;

		return *this;
	}

	inline Vector4D& Negate()
	{
		x = -x;
		y = -y;
		z = -z;
		w = -w;

		return *this;
	}

	inline float Dot(const Vector4D& other)
	{
		return x * other.x + y * other.y + z * other.z + w * other.w;
	}

	inline float LengthSqr()
	{
		return x * x + y * y + z * z + w * w;
	}

	inline float Length()
	{
		return sqrt(LengthSqr());
	}

	inline float Distance(const Vector4D& ToVector)
	{
		return (ToVector - *this).Length();
	}

	inline Vector4D GetNormalize()
	{
		const float flVectorLength = Length();
		float flRecipLength = 1.f;

		if (flVectorLength)
			flRecipLength /= flVectorLength;

		return Vector4D(x * flRecipLength, y * flRecipLength, z * flRecipLength, w * flRecipLength);
	}

	inline float Normalize()
	{
		const float flVectorLength = Length();

		if (flVectorLength)
		{
			float flRecipLength = 1.f / flVectorLength;

			x *= flRecipLength;
			y *= flRecipLength;
			z *= flRecipLength;
			w *= flRecipLength;
		}

		return flVectorLength;
	}

	inline float AngleBetween(Vector4D other)
	{
		other.Normalize();
		Normalize();

		auto angle = RAD2DEG(acos(Dot(other)));

		if (isfinite(angle))
			return angle;

		return 0.f;
	}

	inline Vector4D& operator=(const Vector4D& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;
		w = in.w;

		return *this;
	}

	inline Vector4D& operator=(float* p)
	{
		if (p)
		{
			x = p[0];
			y = p[1];
			z = p[2];
			w = p[3];
		}
		else
		{
			x = 0.f;
			y = 0.f;
			z = 0.f;
			w = 0.f;
		}

		return *this;
	}

	inline Vector4D& operator=(float v)
	{
		x = v;
		y = v;
		z = v;
		w = v;

		return *this;
	}

	inline float& operator[](int i) const
	{
		if (i >= 0 && i < 4)
			return ((float*)this)[i];

		return ((float*)this)[0];
	}

	inline bool operator!() const
	{
		return IsZero();
	}

	inline bool operator==(const Vector4D& other) const
	{
		return x == other.x && y == other.y && z == other.z && w == other.w;
	}

	inline bool operator!=(const Vector4D& other) const
	{
		return x != other.x || y != other.y || z != other.z || w != other.w;
	}

	inline Vector4D& operator+=(const Vector4D& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;
		w += other.w;

		return *this;
	}

	inline Vector4D& operator+=(float* p)
	{
		x += p[0];
		y += p[1];
		z += p[2];
		w += p[3];

		return *this;
	}

	inline Vector4D& operator+=(float v)
	{
		x += v;
		y += v;
		z += v;
		w += v;

		return *this;
	}

	inline Vector4D& operator-=(const Vector4D& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;
		w -= other.w;

		return *this;
	}

	inline Vector4D& operator-=(float* p)
	{
		x -= p[0];
		y -= p[1];
		z -= p[2];
		w -= p[3];

		return *this;
	}

	inline Vector4D& operator-=(float v)
	{
		x -= v;
		y -= v;
		z -= v;
		w -= v;

		return *this;
	}

	inline Vector4D& operator*=(const Vector4D& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;
		w *= other.w;

		return *this;
	}

	inline Vector4D& operator*=(float* p)
	{
		x *= p[0];
		y *= p[1];
		z *= p[2];
		w *= p[3];

		return *this;
	}

	inline Vector4D& operator*=(float v)
	{
		x *= v;
		y *= v;
		z *= v;
		w *= v;

		return *this;
	}

	inline Vector4D& operator/=(const Vector4D& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;
		w /= other.w;

		return *this;
	}

	inline Vector4D& operator/=(float* p)
	{
		x /= p[0];
		y /= p[1];
		z /= p[2];
		w /= p[3];

		return *this;
	}

	inline Vector4D& operator/=(float v)
	{
		x /= v;
		y /= v;
		z /= v;
		w /= v;

		return *this;
	}

	inline Vector4D operator+(const Vector4D& other) const
	{
		return Vector4D(x + other.x, y + other.y, z + other.z, w + other.w);
	}

	inline Vector4D operator+(float* p) const
	{
		return Vector4D(x + p[0], y + p[1], z + p[2], w + p[3]);
	}

	inline Vector4D operator+(float v) const
	{
		return Vector4D(x + v, y + v, z + v, w + v);
	}

	inline Vector4D operator-(const Vector4D& other) const
	{
		return Vector4D(x - other.x, y - other.y, z - other.z, w - other.w);
	}

	inline Vector4D operator-(float* p) const
	{
		return Vector4D(x - p[0], y - p[1], z - p[2], w - p[3]);
	}

	inline Vector4D operator-(float v) const
	{
		return Vector4D(x - v, y - v, z - v, w - v);
	}

	inline Vector4D operator-() const
	{
		return Vector4D(-x, -y, -z, -w);
	}

	inline Vector4D operator*(const Vector4D& other) const
	{
		return Vector4D(x * other.x, y * other.y, z * other.z, w * other.w);
	}

	inline Vector4D operator*(float* p) const
	{
		return Vector4D(x * p[0], y * p[1], z * p[2], w * p[3]);
	}

	inline Vector4D operator*(float v) const
	{
		return Vector4D(x * v, y * v, z * v, w * v);
	}

	inline Vector4D operator/(const Vector4D& other) const
	{
		return Vector4D(x / other.x, y / other.y, z / other.z, w / other.w);
	}

	inline Vector4D operator/(float* p) const
	{
		return Vector4D(x / p[0], y / p[1], z / p[2], w / p[3]);
	}

	inline Vector4D operator/(float v) const
	{
		return Vector4D(x / v, y / v, z / v, w / v);
	}

	operator float* ()
	{
		return &x;
	}

	operator const float* () const
	{
		return &x;
	}
};