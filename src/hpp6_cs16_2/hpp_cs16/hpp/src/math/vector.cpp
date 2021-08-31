#include "framework.h"

void Vector::AngleMatrix(const QAngle& Rotation, float(*matrix)[3])
{
	float radx = Rotation.x * (IM_PI * 2.f / 360.f);
	float rady = Rotation.y * (IM_PI * 2.f / 360.f);
	float radz = Rotation.z * (IM_PI * 2.f / 360.f);

	float sp = sin(radx);
	float sy = sin(rady);
	float sr = sin(radz);

	float cp = cos(radx);
	float cy = cos(rady);
	float cr = cos(radz);

	matrix[0][0] = cp * cy;
	matrix[0][1] = cp * sy;
	matrix[0][2] = -sp;

	matrix[1][0] = 1.f * sr * sp * cy + 1.f * cr * -sy;
	matrix[1][1] = 1.f * sr * sp * sy + 1.f * cr * cy;
	matrix[1][2] = 1.f * sr * cp;

	matrix[2][0] = cr * sp * cy + -sr * -sy;
	matrix[2][1] = cr * sp * sy + -sr * cy;
	matrix[2][2] = cr * cp;
}

void Vector::VectorRotate(Vector& vecInput, const QAngle& QRotation)
{
	assert(vecInput != *this);

	float flMatRotate[3][3];
	AngleMatrix(QRotation, flMatRotate);

	x = vecInput.Dot(flMatRotate[0]);
	y = vecInput.Dot(flMatRotate[1]);
	z = vecInput.Dot(flMatRotate[2]);
}

/*QAngle Vector::ToEulerAngles()
{
	float flPitch, flYaw;
	float flLength = Length2D();

	if (flLength > 0.f) 
	{
		flPitch = RAD2DEG(atan2(-z, flLength));

		if (flPitch < 0.f)
			flPitch += 360.f;

		flYaw = RAD2DEG(atan2(y, x));

		if (flYaw < 0.f)
			flYaw += 360.f;
	}
	else 
	{
		flPitch = (z > 0.f) ? 270.f : 90.f;
		flYaw = 0.f;
	}

	return QAngle(flPitch, flYaw, 0.f);
}*/

QAngle Vector::ToEulerAngles(Vector* vecPseudoUp)
{
	float flYaw, flPitch, flRoll;
	float flLength = Length2D();

	if (vecPseudoUp) 
	{
		Vector vecLeft;
		vecLeft.CrossProduct(*vecPseudoUp, *this);
		vecLeft.Normalize();

		if (flLength > 0.001f) 
		{
			flPitch = RAD2DEG(atan2(-z, flLength));

			if (flPitch < 0.f)
				flPitch += 360.f;

			flYaw = RAD2DEG(atan2(y, x));

			if (flYaw < 0.f)
				flYaw += 360.f;

			float up_z = (vecLeft[1] * x) - (vecLeft[0] * y);

			flRoll = RAD2DEG(atan2(vecLeft[2], up_z));

			if (flRoll < 0.f)
				flRoll += 360.f;
		}
		else 
		{
			flYaw = RAD2DEG(atan2(y, x));

			if (flYaw < 0.f)
				flYaw += 360.f;

			flPitch = RAD2DEG(atan2(-z, flLength));

			if (flPitch < 0.f)
				flPitch += 360.f;

			flRoll = 0.f;
		}
	}
	else 
	{
		if (flLength > 0.f)
		{
			flPitch = RAD2DEG(atan2(-z, flLength));

			if (flPitch < 0.f)
				flPitch += 360.f;

			flYaw = RAD2DEG(atan2(y, x));

			if (flYaw < 0.f)
				flYaw += 360.f;
		}
		else 
		{
			flPitch = (z > 0.f) ? 270.f : 90.f;
			flYaw = 0.f;
		}

		flRoll = 0.f;
	}

	return QAngle(flPitch, flYaw, flRoll);
}