#pragma once

class QAngle 
{
public:
	float x, y, z;

	QAngle() :
		x(0),
		y(0),
		z(0)
	{
	}

	QAngle(float _x, float _y, float _z) : 
		x(_x),
		y(_y),
		z(_z)
	{
	}

	QAngle(float v) :
		x(v),
		y(v),
		z(v)
	{
	}

	QAngle(const Vector& v) :
		x(v.x),
		y(v.y),
		z(v.z)
	{
	}

	QAngle(float* p)
	{
		*this = p;
	};

	QAngle(QAngle& in)
	{
		*this = in;
	};

	~QAngle()
	{
	}

	inline bool IsZero() const
	{
		return x == 0 && y == 0 && z == 0;
	}

	inline bool IsZero2D()
	{
		return x == 0 && y == 0;
	}

	inline QAngle& Clear()
	{
		x = 0.f;
		y = 0.f;
		z = 0.f;

		return *this;
	}

	inline QAngle& Init(float _x, float _y, float _z)
	{
		x = _x;
		y = _y;
		z = _z;

		return *this;
	}

	inline QAngle& Init(const Vector& v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	inline QAngle& Init(float* p)
	{
		*this = p;

		return *this;
	}

	inline QAngle& Negate()
	{
		x = -x;
		y = -y;
		z = -z;

		return *this;
	}

	inline QAngle& Normalize()
	{
		float* p = (float*)this;

		for (int i = 0; i < 3; i++) 
		{
			if (p[i] > 180.f)
				p[i] -= 360.f;
			
			if (p[i] < -180.f)
				p[i] += 360.f;
		}

		return *this;
	}

	inline QAngle Delta360(QAngle& other)
	{
		QAngle OutDifference = other - *this;

		for (int i = 0; i < 3; i++) 
		{
			if (OutDifference[i] < 0) 
			{
				OutDifference[i] = -OutDifference[i];

				if (OutDifference[i] > 180.f)
					OutDifference[i] = 360.f - OutDifference[i];
			}
		}

		return OutDifference;
	}

	inline bool IsWithinFOV(QAngle& AngIn, float fovX, float fovY)
	{
		QAngle AngleDifference = Delta360(AngIn);

		return AngleDifference.x <= fovX && AngleDifference.y <= fovY;
	}

	inline QAngle& operator=(const QAngle& in)
	{
		x = in.x;
		y = in.y;
		z = in.z;

		return *this;
	}

	inline QAngle& operator=(float* p)
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

	inline QAngle& operator=(float v)
	{
		x = v;
		y = v;
		z = v;

		return *this;
	}

	inline QAngle& operator=(Vector v)
	{
		x = v.x;
		y = v.y;
		z = v.z;

		return *this;
	}

	inline float& operator[](int i) const
	{
		if (i >= 0 && i < 3)
			return ((float*)this)[i];

		return ((float*)this)[0];
	}

	inline bool operator!(void)const
	{
		return IsZero();
	}

	inline bool operator==(const QAngle& other) const
	{
		return x == other.x && y == other.y && z == other.z;
	}

	inline bool operator!=(const QAngle& other) const
	{
		return x != other.x || y != other.y || z != other.z;
	}

	inline QAngle& operator+=(const QAngle& other)
	{
		x += other.x;
		y += other.y;
		z += other.z;

		return *this;
	}

	inline QAngle& operator+=(float* p)
	{
		x += p[0];
		y += p[1];
		z += p[2];

		return *this;
	}

	inline QAngle& operator+=(float v)
	{
		x += v;
		y += v;
		z += v;

		return *this;
	}

	inline QAngle& operator-=(const QAngle& other)
	{
		x -= other.x;
		y -= other.y;
		z -= other.z;

		return *this;
	}

	inline QAngle& operator-=(float* p)
	{
		x -= p[0];
		y -= p[1];
		z -= p[2];

		return *this;
	}
	inline QAngle& operator-=(float v)
	{
		x -= v;
		y -= v;
		z -= v;

		return *this;
	}

	inline QAngle& operator*=(const QAngle& other)
	{
		x *= other.x;
		y *= other.y;
		z *= other.z;

		return *this;
	}

	inline QAngle& operator*=(float* p)
	{
		x *= p[0];
		y *= p[1];
		z *= p[2];

		return *this;
	}

	inline QAngle& operator*=(float v)
	{
		x *= v;
		y *= v;
		z *= v;

		return *this;
	}

	inline QAngle& operator/=(const QAngle& other)
	{
		x /= other.x;
		y /= other.y;
		z /= other.z;

		return *this;
	}

	inline QAngle& operator/=(float* p)
	{
		x /= p[0];
		y /= p[1];
		z /= p[2];

		return *this;
	}

	inline QAngle& operator/=(float v)
	{
		x /= v;
		y /= v;
		z /= v;

		return *this;
	}

	inline QAngle operator+(const QAngle& other) const
	{
		return QAngle(x + other.x, y + other.y, z + other.z);
	}

	inline QAngle operator+(float* p) const
	{
		return QAngle(x + p[0], y + p[1], z + p[2]);
	}

	inline QAngle operator+(float v) const
	{
		return QAngle(x + v, y + v, z + v);
	}

	inline QAngle operator-(const QAngle& other) const
	{
		return QAngle(x - other.x, y - other.y, z - other.z);
	}

	inline QAngle operator-(float* p) const
	{
		return QAngle(x - p[0], y - p[1], z - p[2]);
	}

	inline QAngle operator-(float v) const
	{
		return QAngle(x - v, y - v, z - v);
	}

	inline QAngle operator-() const
	{
		return QAngle(-x, -y, -z);
	}

	inline QAngle operator*(const QAngle& other) const
	{
		return QAngle(x * other.x, y * other.y, z * other.z);
	}

	inline QAngle operator*(float* p) const
	{
		return QAngle(x * p[0], y * p[1], z * p[2]);
	}

	inline QAngle operator*(float v) const
	{
		return QAngle(x * v, y * v, z * v);
	}

	inline QAngle operator/(const QAngle& other) const
	{
		return QAngle(x / other.x, y / other.y, z / other.z);
	}

	inline QAngle operator/(float* p) const
	{
		return QAngle(x / p[0], y / p[1], z / p[2]);
	}

	inline QAngle operator/(float v) const
	{
		return QAngle(x / v, y / v, z / v);
	}

	operator float* () 
	{ 
		return &x; 
	}

	operator const float* () const 
	{ 
		return &x; 
	}

	void AngleVectors(Vector* Forward, Vector* Right, Vector* Up);
	void AngleVectorsTranspose(Vector* Forward, Vector* Right, Vector* Up);
};









