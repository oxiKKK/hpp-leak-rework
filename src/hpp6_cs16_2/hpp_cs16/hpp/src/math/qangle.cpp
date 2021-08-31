#include "framework.h"

void QAngle::AngleVectors(Vector* Forward, Vector* Right, Vector* Up)
{
	float radx = x * (IM_PI * 2.f / 360.f);
	float rady = y * (IM_PI * 2.f / 360.f);
	float radz = z * (IM_PI * 2.f / 360.f);

	float sp = sin(radx);
	float sy = sin(rady);
	float sr = sin(radz);

	float cp = cos(radx);
	float cy = cos(rady);
	float cr = cos(radz);

	if (Forward) 
	{
		Forward->x = cp * cy;
		Forward->y = cp * sy;
		Forward->z = -sp;
	}

	if (Right) 
	{
		Right->x = -1.f * sr * sp * cy + -1.f * cr * -sy;
		Right->y = -1.f * sr * sp * sy + -1.f * cr * cy;
		Right->z = -1.f * sr * cp;
	}

	if (Up) 
	{
		Up->x = cr * sp * cy + -sr * -sy;
		Up->y = cr * sp * sy + -sr * cy;
		Up->z = cr * cp;
	}
}

void QAngle::AngleVectorsTranspose(Vector* Forward, Vector* Right, Vector* Up)
{
	float radx = x * (IM_PI * 2.f / 360.f);
	float rady = y * (IM_PI * 2.f / 360.f);
	float radz = z * (IM_PI * 2.f / 360.f);

	float sp = sin(radx);
	float sy = sin(rady);
	float sr = sin(radz);

	float cp = cos(radx);
	float cy = cos(rady);
	float cr = cos(radz);

	if (Forward) 
	{
		Forward->x = cp * cy;
		Forward->y = sr * sp * cy + cr * -sy;
		Forward->z = cr * sp * cy + -sr * -sy;
	}

	if (Right) 
	{
		Right->x = cp * sy;
		Right->y = sr * sp * sy + cr * cy;
		Right->z = cr * sp * sy + -sr * cy;
	}

	if (Up) 
	{
		Up->x = -sp;
		Up->y = sr * cp;
		Up->z = cr * cp;
	}
}