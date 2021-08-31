#include "main.h"

void Vector::AngleMatrix(QAngle& Rotation, float(*matrix)[3])
{
	float radx = Rotation.x * (float)(M_PI * 2 / 360);
	float rady = Rotation.y * (float)(M_PI * 2 / 360);
	float radz = Rotation.z * (float)(M_PI * 2 / 360);

	float sp = sinf(radx);
	float sy = sinf(rady);
	float sr = sinf(radz);

	float cp = cosf(radx);
	float cy = cosf(rady);
	float cr = cosf(radz);

	matrix[0][0] = cp * cy;
	matrix[0][1] = cp * sy;
	matrix[0][2] = -sp;

	matrix[1][0] = 1 * sr * sp * cy + 1 * cr * -sy;
	matrix[1][1] = 1 * sr * sp * sy + 1 * cr * cy;
	matrix[1][2] = 1 * sr * cp;

	matrix[2][0] = cr * sp * cy + -sr * -sy;
	matrix[2][1] = cr * sp * sy + -sr * cy;
	matrix[2][2] = cr * cp;
}

void Vector::VectorRotate(Vector& In, QAngle& Rotation)
{
	float flMatRotate[3][3];
	AngleMatrix(Rotation, flMatRotate);

	x = In.Dot(flMatRotate[0]);
	y = In.Dot(flMatRotate[1]);
	z = In.Dot(flMatRotate[2]);
}

QAngle Vector::ToEulerAngles()
{
	float Pitch, Yaw;
	float Length = Length2D();

	if (Length > 0) {
		Pitch = RAD2DEG(atan2f(-z, Length));
		;

		if (Pitch < 0)
			Pitch += 360;

		Yaw = RAD2DEG(atan2f(y, x));

		if (Yaw < 0)
			Yaw += 360;
	}
	else {
		Pitch = (z > 0) ? 270.f : 90.f;
		Yaw = 0;
	}

	return QAngle(Pitch, Yaw, 0);
}

QAngle Vector::ToEulerAngles(Vector* PseudoUp)
{
	float Yaw, Pitch, Roll;
	float Length = Length2D();

	Vector Left;
	Left.CrossProduct(*PseudoUp, *this);
	Left.Normalize();

	if (PseudoUp) {
		if (Length > 0.001) {
			Pitch = RAD2DEG(atan2f(-z, Length));

			if (Pitch < 0)
				Pitch += 360;

			Yaw = RAD2DEG(atan2f(y, x));

			if (Yaw < 0)
				Yaw += 360;

			float up_z = (Left[1] * x) - (Left[0] * y);

			Roll = RAD2DEG(atan2f(Left[2], up_z));

			if (Roll < 0)
				Roll += 360;
		}
		else {
			Yaw = RAD2DEG(atan2f(y, x));

			if (Yaw < 0)
				Yaw += 360;

			Pitch = RAD2DEG(atan2f(-z, Length));

			if (Pitch < 0)
				Pitch += 360;

			Roll = 0;
		}
	}
	else {
		if (Length > 0) {
			Pitch = RAD2DEG(atan2f(-z, Length));

			if (Pitch < 0)
				Pitch += 360;

			Yaw = RAD2DEG(atan2f(y, x));

			if (Yaw < 0)
				Yaw += 360;
		}
		else {
			Pitch = (z > 0) ? 270.f : 90.f;
			Yaw = 0;
		}
	}

	return QAngle(Pitch, Yaw, Roll);
}