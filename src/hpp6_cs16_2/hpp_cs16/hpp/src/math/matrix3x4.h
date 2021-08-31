struct matrix3x4_t
{
	matrix3x4_t() { Clear(); }
	matrix3x4_t(const float(&m)[3][4])
	{
		m_flMatVal[0][0] = m[0][0];	m_flMatVal[0][1] = m[0][1]; m_flMatVal[0][2] = m[0][2]; m_flMatVal[0][3] = m[0][3];
		m_flMatVal[1][0] = m[1][0];	m_flMatVal[1][1] = m[1][1]; m_flMatVal[1][2] = m[1][2]; m_flMatVal[1][3] = m[1][3];
		m_flMatVal[2][0] = m[2][0];	m_flMatVal[2][1] = m[2][1]; m_flMatVal[2][2] = m[2][2]; m_flMatVal[2][3] = m[2][3];
	}
	matrix3x4_t(
		float m00, float m01, float m02, float m03,
		float m10, float m11, float m12, float m13,
		float m20, float m21, float m22, float m23)
	{
		m_flMatVal[0][0] = m00;	m_flMatVal[0][1] = m01; m_flMatVal[0][2] = m02; m_flMatVal[0][3] = m03;
		m_flMatVal[1][0] = m10;	m_flMatVal[1][1] = m11; m_flMatVal[1][2] = m12; m_flMatVal[1][3] = m13;
		m_flMatVal[2][0] = m20;	m_flMatVal[2][1] = m21; m_flMatVal[2][2] = m22; m_flMatVal[2][3] = m23;
	}

	inline void SetOrigin(Vector const& p)
	{
		m_flMatVal[0][3] = p.x;
		m_flMatVal[1][3] = p.y;
		m_flMatVal[2][3] = p.z;
	}

	inline Vector GetOrigin()
	{
		return Vector(m_flMatVal[0][3], m_flMatVal[1][3], m_flMatVal[2][3]);
	}

	inline void Clear()
	{
		m_flMatVal[0][0] = 0; m_flMatVal[0][1] = 0; m_flMatVal[0][2] = 0; m_flMatVal[0][3] = 0;
		m_flMatVal[1][0] = 0; m_flMatVal[1][1] = 0; m_flMatVal[1][2] = 0; m_flMatVal[1][3] = 0;
		m_flMatVal[2][0] = 0; m_flMatVal[2][1] = 0; m_flMatVal[2][2] = 0; m_flMatVal[2][3] = 0;
	}

	float* operator[](int i) { assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	const float* operator[](int i) const { assert((i >= 0) && (i < 3)); return m_flMatVal[i]; }
	float* Base() { return &m_flMatVal[0][0]; }
	const float* Base() const { return &m_flMatVal[0][0]; }

	float m_flMatVal[3][4];
};