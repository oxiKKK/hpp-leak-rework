#include "main.h"

void QAngle::AngleVectors(Vector* Forward, Vector* Right, Vector* Up)
{
	float radx = x * (float)(M_PI * 2 / 360);
	float rady = y * (float)(M_PI * 2 / 360);
	float radz = z * (float)(M_PI * 2 / 360);

	float sp = sinf(radx);
	float sy = sinf(rady);
	float sr = sinf(radz);

	float cp = cosf(radx);
	float cy = cosf(rady);
	float cr = cosf(radz);

	if (Forward) {
		Forward->x = cp * cy;
		Forward->y = cp * sy;
		Forward->z = -sp;
	}

	if (Right) {
		Right->x = -1 * sr * sp * cy + -1 * cr * -sy;
		Right->y = -1 * sr * sp * sy + -1 * cr * cy;
		Right->z = -1 * sr * cp;
	}

	if (Up) {
		Up->x = cr * sp * cy + -sr * -sy;
		Up->y = cr * sp * sy + -sr * cy;
		Up->z = cr * cp;
	}
}

void QAngle::AngleVectorsTranspose(Vector* Forward, Vector* Right, Vector* Up)
{
	float radx = x * (float)(M_PI * 2 / 360);
	float rady = y * (float)(M_PI * 2 / 360);
	float radz = z * (float)(M_PI * 2 / 360);

	float sp = sinf(radx);
	float sy = sinf(rady);
	float sr = sinf(radz);

	float cp = cosf(radx);
	float cy = cosf(rady);
	float cr = cosf(radz);

	if (Forward) {
		Forward->x = cp * cy;
		Forward->y = sr * sp * cy + cr * -sy;
		Forward->z = cr * sp * cy + -sr * -sy;
	}

	if (Right) {
		Right->x = cp * sy;
		Right->y = sr * sp * sy + cr * cy;
		Right->z = cr * sp * sy + -sr * cy;
	}

	if (Up) {
		Up->x = -sp;
		Up->y = sr * cp;
		Up->z = cr * cp;
	}
}