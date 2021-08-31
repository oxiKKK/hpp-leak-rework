#include "framework.h"

void Math::VectorITransform(const Vector& vecInput, const matrix3x4_t& in2, Vector& vecOutput)
{
	Vector vecTemp;

	vecTemp[0] = vecInput[0] - in2[0][3];
	vecTemp[1] = vecInput[1] - in2[1][3];
	vecTemp[2] = vecInput[2] - in2[2][3];

	vecOutput[0] = vecTemp[0] * in2[0][0] + vecTemp[1] * in2[1][0] + vecTemp[2] * in2[2][0];
	vecOutput[1] = vecTemp[0] * in2[0][1] + vecTemp[1] * in2[1][1] + vecTemp[2] * in2[2][1];
	vecOutput[2] = vecTemp[0] * in2[0][2] + vecTemp[1] * in2[1][2] + vecTemp[2] * in2[2][2];
}

void Math::VectorIRotate(const Vector& vecInput, const matrix3x4_t& in2, Vector& vecOutput)
{
	assert(vecInput != vecOutput);

	vecOutput[0] = vecInput[0] * in2[0][0] + vecInput[1] * in2[1][0] + vecInput[2] * in2[2][0];
	vecOutput[1] = vecInput[0] * in2[0][1] + vecInput[1] * in2[1][1] + vecInput[2] * in2[2][1];
	vecOutput[2] = vecInput[0] * in2[0][2] + vecInput[1] * in2[1][2] + vecInput[2] * in2[2][2];
}

bool Math::IntersectRayWithBox(const Vector& rayStart, const Vector& rayDelta, const Vector& boxMins, const Vector& boxMaxs, float& fraction, int& hitside, bool& startsolid)
{
	float enterfrac, leavefrac, d1, d2, f;

	hitside = -1;
	enterfrac = -1.0f;
	leavefrac = 1.0f;

	Vector rayEnd = rayStart + rayDelta;

	startsolid = true;

	for (int i = 0; i < 6; i++)
	{
		if (i >= 3)
		{
			d1 = rayStart[i - 3] - boxMaxs[i - 3];
			d2 = rayEnd[i - 3] - boxMaxs[i - 3];
		}
		else
		{
			d1 = -rayStart[i] + boxMins[i];
			d2 = -rayEnd[i] + boxMins[i];
		}

		// if completely in front of face, no intersection
		if (d1 > 0 && d2 > 0)
			return false;

		// completely inside, check next face
		if (d1 <= 0 && d2 <= 0)
			continue;

		if (d1 > 0)
			startsolid = false;

		// crosses face
		if (d1 > d2)
		{
			f = d1;
			if (f < 0)
				f = 0;
			f = f / (d1 - d2);
			if (f > enterfrac)
			{
				enterfrac = f;
				hitside = i;
			}
		}
		else
		{ // leave
			f = d1 / (d1 - d2);
			if (f < leavefrac)
			{
				leavefrac = f;
			}
		}
	}
	if (enterfrac < leavefrac && enterfrac >= 0.0f)
	{
		fraction = enterfrac;
		return true;
	}

	if (startsolid)
	{
		fraction = 0;
		hitside = 0;
		return true;
	}

	return false;
}

bool Math::IntersectRayWithOBB(const Vector& vecRayStart, const Vector& vecRayDelta, const matrix3x4_t& matOBBToWorld, const Vector& vecOBBMins, const Vector& vecOBBMaxs, float& fraction, int& hitside, bool& startsolid)
{
	Vector vecStart, vecDelta;
	VectorITransform(vecRayStart, matOBBToWorld, vecStart);
	VectorIRotate(vecRayDelta, matOBBToWorld, vecDelta);

	return IntersectRayWithBox(vecStart, vecDelta, vecOBBMins, vecOBBMaxs, fraction, hitside, startsolid);
}

void Math::VectorTransform(Vector vecInput, matrix3x4_t matrix, const Vector& vecOutput)
{
	vecOutput[0] = vecInput.Dot(matrix[0]) + matrix[0][3];
	vecOutput[1] = vecInput.Dot(matrix[1]) + matrix[1][3];
	vecOutput[2] = vecInput.Dot(matrix[2]) + matrix[2][3];
}

void Math::VectorTransform(Vector vecInput, matrix3x4_t matrix, const Vector& vecOutput, const int &xyz, const float &multi)
{
	vecInput[xyz] *= multi;

	vecOutput[0] = vecInput.Dot(matrix[0]) + matrix[0][3];
	vecOutput[1] = vecInput.Dot(matrix[1]) + matrix[1][3];
	vecOutput[2] = vecInput.Dot(matrix[2]) + matrix[2][3];
}

void Math::ComputeMove(Vector vecVelocity, const QAngle& QAngles, float* forwardmove, float* sidemove)
{
	assert(forwardmove && sidemove);

	if (!vecVelocity.IsZero2D())
	{
		Vector vecVelocityRotated;
		vecVelocityRotated.VectorRotate(vecVelocity, QAngles);

		*forwardmove = vecVelocityRotated.x;
		*sidemove = -1.f * vecVelocityRotated.y;
	}
}

void Math::NormalizeAngles(const QAngle& QAngles)
{
	for (int i = 0; i < 3; i++)
	{
		float flRevolutions = QAngles[i] / 360.f;

		if (QAngles[i] > 180.f || QAngles[i] < -180.f)
		{
			if (flRevolutions < 0.f)
				flRevolutions = -flRevolutions;

			flRevolutions = round(flRevolutions);

			if (QAngles[i] < 0.f)
				QAngles[i] = QAngles[i] + 360.f * flRevolutions;
			else
				QAngles[i] = QAngles[i] - 360.f * flRevolutions;
		}
	}
}

void Math::VectorAngles(const Vector &vecForward, QAngle& QAngles)
{
	float flYaw, flPitch;

	if (vecForward.IsZero2D())
	{
		flYaw = 0.f;
		flPitch = vecForward.z > 0.f ? 270.f : 90.f;
	}
	else
	{
		flYaw = (atan2(vecForward.y, vecForward.x) * 180.f / IM_PI);

		if (flYaw < 0.f)
			flYaw += 360.f;

		float flTemp = sqrt(vecForward.x * vecForward.x + vecForward.y * vecForward.y);
		flPitch = atan2(-vecForward.z, flTemp) * 180.f / IM_PI;

		if (flPitch < 0.f)
			flPitch += 360.f;
	}

	QAngles[0] = flPitch;
	QAngles[1] = flYaw;
	QAngles[2] = 0.f;

	while (QAngles[0] < -89.f)
	{
		QAngles[0] += 180.f;
		QAngles[1] += 180.f;
	}

	while (QAngles[0] > 89.f)
	{
		QAngles[0] -= 180.f;
		QAngles[1] += 180.f;
	}

	while (QAngles[1] < -180.f)
		QAngles[1] += 360.f;

	while (QAngles[1] > 180.f)
		QAngles[1] -= 360.f;
}

double Math::Interp(double s1, double s2, double s3, double f1, double f3)
{
	if (s2 == s1)
		return f1;

	if (s2 == s3)
		return f3;

	if (s3 == s1)
		return f1;

	return f1 + ((s2 - s1) / (s3 - s1)) * (f3 - f1);
}

double Math::Interp_F2(double s1, double s2, double s3, double f1, double f3)
{
	return f1 + ((s2 - s1) / s3) * (f3 - f1);
}

double Math::Interp_S2(double s1, double s3, double f1, double f2, double f3)
{
	return ((f2 - f1) + s1 / s3  * (f3 - f1)) / (f3 - f1) * s3;
}

float Math::TranslateToEvenGreatest(float val)
{
	return round(val * 0.5f) * 2.f;
}