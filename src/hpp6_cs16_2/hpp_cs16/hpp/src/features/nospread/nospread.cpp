#include "framework.h"

std::unique_ptr<CNoSpread> g_pNoSpread;

unsigned glSeed = 0U;

unsigned seed_table[256] = 
{
	28985U, 27138U, 26457U, 9451U, 17764U, 10909U, 28790U, 8716U, 6361U, 4853U, 17798U, 21977U, 19643U, 20662U, 10834U, 20103U,
	27067U, 28634U, 18623U, 25849U, 8576U, 26234U, 23887U, 18228U, 32587U, 4836U, 3306U, 1811U, 3035U, 24559U, 18399U, 315U,
	26766U, 907U, 24102U, 12370U, 9674U, 2972U, 10472U, 16492U, 22683U, 11529U, 27968U, 30406U, 13213U, 2319U, 23620U, 16823U,
	10013U, 23772U, 21567U, 1251U, 19579U, 20313U, 18241U, 30130U, 8402U, 20807U, 27354U, 7169U, 21211U, 17293U, 5410U, 19223U,
	10255U, 22480U, 27388U, 9946U, 15628U, 24389U, 17308U, 2370U, 9530U, 31683U, 25927U, 23567U, 11694U, 26397U, 32602U, 15031U,
	18255U, 17582U, 1422U, 28835U, 23607U, 12597U, 20602U, 10138U, 5212U, 1252U, 10074U, 23166U, 19823U, 31667U, 5902U, 24630U,
	18948U, 14330U, 14950U, 8939U, 23540U, 21311U, 22428U, 22391U, 3583U, 29004U, 30498U, 18714U, 4278U, 2437U, 22430U, 3439U,
	28313U, 23161U, 25396U, 13471U, 19324U, 15287U, 2563U, 18901U, 13103U, 16867U, 9714U, 14322U, 15197U, 26889U, 19372U, 26241U,
	31925U, 14640U, 11497U, 8941U, 10056U, 6451U, 28656U, 10737U, 13874U, 17356U, 8281U, 25937U, 1661U, 4850U, 7448U, 12744U,
	21826U, 5477U, 10167U, 16705U, 26897U, 8839U, 30947U, 27978U, 27283U, 24685U, 32298U, 3525U, 12398U, 28726U, 9475U, 10208U,
	617U, 13467U, 22287U, 2376U, 6097U, 26312U, 2974U, 9114U, 21787U, 28010U, 4725U, 15387U, 3274U, 10762U, 31695U, 17320U,
	18324U, 12441U, 16801U, 27376U, 22464U, 7500U, 5666U, 18144U, 15314U, 31914U, 31627U, 6495U, 5226U, 31203U, 2331U, 4668U,
	12650U, 18275U, 351U, 7268U, 31319U, 30119U, 7600U, 2905U, 13826U, 11343U, 13053U, 15583U, 30055U, 31093U, 5067U, 761U,
	9685U, 11070U, 21369U, 27155U, 3663U, 26542U, 20169U, 12161U, 15411U, 30401U, 7580U, 31784U, 8985U, 29367U, 20989U, 14203U,
	29694U, 21167U, 10337U, 1706U, 28578U, 887U, 3373U, 19477U, 14382U, 675U, 7033U, 15111U, 26138U, 12252U, 30996U, 21409U,
	25678U, 18555U, 13256U, 23316U, 22407U, 16727U, 991U, 9236U, 5373U, 29402U, 6117U, 15241U, 27715U, 19291U, 19888U, 19847U
};

unsigned U_Random()
{
	glSeed *= 69069;
	glSeed += seed_table[glSeed & 0xFF] + 1;
	return (glSeed & 0xFFFFFFF);
}

void U_Srand(unsigned seed)
{
	glSeed = seed_table[seed & 0xFF];
}

int UTIL_SharedRandomLong(unsigned seed, int low, int high)
{
	const auto range = (unsigned)(high - low + 1);
	U_Srand((unsigned)(high + low + seed));

	if (range != 1) 
	{
		const int rnum = U_Random();
		const int offset = rnum % range;
		return (low + offset);
	}

	return low;
}

float UTIL_SharedRandomFloat(unsigned seed, float low, float high)
{
	const auto range = (unsigned)(high - low);
	U_Srand((unsigned)seed + *(unsigned*)&low + *(unsigned*)&high);

	U_Random();
	U_Random();

	if (range)
	{
		const int tensixrand = U_Random() & 0xFFFFu;
		const float offset = float(tensixrand) / 0x10000u;
		return (low + offset * range);
	}

	return low;
}

double CNoSpread::GetConstantRemainder(const Vector& vecSpread) // aka Trap God Constant
{
	QAngle QOutAngles, QInAngles(90.f, 0.f, 0.f);

	Vector vecForward, vecRight, vecUp, vecSpreadAt90Pitch(vecSpread.y, -vecSpread.x, -1.f);

	vecSpreadAt90Pitch.Normalize();

	QInAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

	QOutAngles = vecSpreadAt90Pitch.ToEulerAngles(&vecUp);

	QOutAngles.Normalize();

	double ConstantRemainder = (double)QOutAngles.y - (double)QOutAngles.z;

	return ConstantRemainder;
}

void CNoSpread::GetSpreadXY(const unsigned& random_seed, const int& future, Vector& vecSpread)
{
	vecSpread.x = UTIL_SharedRandomFloat(random_seed + future, -0.5f, 0.5f) + UTIL_SharedRandomFloat(random_seed + 1 + future, -0.5f, 0.5f);
	vecSpread.y = UTIL_SharedRandomFloat(random_seed + 2 + future, -0.5f, 0.5f) + UTIL_SharedRandomFloat(random_seed + 3 + future, -0.5f, 0.5f);

	vecSpread.x *= g_Weapon->m_flSpread;
	vecSpread.y *= g_Weapon->m_flSpread;
}

#pragma warning(disable: 4244)

void CNoSpread::GetSpreadOffset(const unsigned& random_seed, const int& future, const QAngle& QInAngles, QAngle& QOutAngles, const int &type)
{
	QAngle QAngles, QNewAngles;

	QAngles = QInAngles;

	QAngles.Normalize();

	QNewAngles = QAngles;

	if (type > NOSPREAD_NONE)
	{
		QAngle QAdjusterAngles;

		Vector vecSpread, vecForward, vecRight, vecUp;

		GetSpreadXY(random_seed, future, vecSpread);

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		float dot = vecSpread.x / sqrt(1.f + vecSpread.x * vecSpread.x + vecSpread.y * vecSpread.y);
		float a = vecForward.y * vecForward.y;

		if (a * (vecForward.x * vecForward.x + a - dot * dot) < 0.f && (type == NOSPREAD_PITCH_ROLL || type == NOSPREAD_PITCH_YAW_ROLL))
		{
			//apply trapgod constant or watch how your pasted iterative nospread fails
			QAdjusterAngles.x = RAD2DEG(atan(sqrt(vecSpread.x * vecSpread.x + vecSpread.y * vecSpread.y)));
			QAdjusterAngles.z = -RAD2DEG(atan2(vecSpread.x, vecSpread.y));

			QNewAngles.x += QAdjusterAngles.x;
			QNewAngles.z = QAdjusterAngles.z;
		}
		else if (type == NOSPREAD_PITCH_YAW || type == NOSPREAD_PITCH_YAW_ROLL)
		{
			Vector vecSpreadAtOriginAngles = Vector(1.f, -vecSpread.x, vecSpread.y).GetNormalize();

			float LengthXZ = sqrt(1.f - vecSpreadAtOriginAngles.y * vecSpreadAtOriginAngles.y);

			QAdjusterAngles.x = -(QAngles.x + RAD2DEG(asin(vecForward.z / LengthXZ) - atan(vecSpread.y)));
			QAdjusterAngles.y = RAD2DEG(atan2(vecSpreadAtOriginAngles.y, sqrt(LengthXZ * LengthXZ - vecForward.z * vecForward.z)));

			QNewAngles.x += QAdjusterAngles.x;
			QNewAngles.y -= QAdjusterAngles.y;
		}
	}

	QNewAngles.Normalize();

	QOutAngles = QNewAngles;
	/*else if (type == NOSPREAD_PITCH_YAW || type == NOSPREAD_PITCH_YAW_ROLL)
	{
		QAngle QAngles, QOldAngles, QAdjusterAngles, QInputAngles, QNewAngles, QTestAngles, QTempPitch, QInputRoll;

		Vector vecSpread, vecForward, vecRight, vecUp, vecDirection, vecInputRight, vecInputRight2, vecRotatedPitch;

		double PitchBreakingPoint, UpVal, CosineInput, CosinePitch, InputRoll, Yaw_1, ReciprocalYaw_1, ReciprocalYaw_2, PitchInput, PitchCosine, PitchSine, TempPitch, ConstantRemainder;

		GetSpreadXY(random_seed, future, vecSpread);

		ConstantRemainder = GetConstantRemainder(vecSpread);

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecDirection = vecForward + (vecRight * -vecSpread.x) + (vecUp * -vecSpread.y);
		vecDirection.Normalize();

		PitchBreakingPoint = RAD2DEG(atan2(vecDirection.y, sqrt(1.f - vecDirection.y * vecDirection.y)));

		if (PitchBreakingPoint > 180.0)
			PitchBreakingPoint -= 360.0;
		else if (PitchBreakingPoint < -180.0)
			PitchBreakingPoint += 360.0;

		PitchBreakingPoint = PitchBreakingPoint < 0.0 ? 90.0 + PitchBreakingPoint : 90.0 - PitchBreakingPoint;

		QAngles = QInAngles; QOldAngles = QAngles;
		QAngles = vecDirection.ToEulerAngles();
		QAngles.Normalize();
		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);
		QAngles = QOldAngles;

		UpVal = vecUp.z;
		CosineInput = QAngles.x * (IM_PI * 2.0 / 360.0);
		CosinePitch = cos(CosineInput);
		Yaw_1 = CosinePitch ? 1.0 / CosinePitch : 0.0;
		Yaw_1 *= vecDirection.y;
		vecInputRight.y = Yaw_1;

		if (Yaw_1 >= 1.0 || Yaw_1 <= -1.0)
		{
			vecInputRight.y = 1.0 / Yaw_1;
			vecInputRight.x = 0.f;
		}
		else
			vecInputRight.x = sqrt(1.f - vecInputRight.y * vecInputRight.y);

		vecInputRight.z = 0.f;
		vecInputRight2.y = vecDirection.y;

		QAdjusterAngles.y = RAD2DEG(atan2(vecInputRight.y, vecInputRight.x));
		QAdjusterAngles.Normalize();

		if (vecDirection.y >= 1.f || vecDirection.y <= -1.f)
		{
			vecInputRight2.y = 1.f / vecDirection.y;
			vecInputRight2.x = 0.f;
		}
		else
			vecInputRight2.x = sqrt(1.f - vecInputRight2.y * vecInputRight2.y);

		ReciprocalYaw_1 = vecInputRight.x ? vecInputRight.y / vecInputRight.x : 0.0;
		ReciprocalYaw_2 = vecInputRight2.x ? vecInputRight2.y / vecInputRight2.x : 0.0;
		PitchInput = 0.0;

		if (QAngles.x)
		{
			if (ReciprocalYaw_1 && ReciprocalYaw_2)
			{
				PitchInput = 1.0;

				if (abs(ReciprocalYaw_1) < abs(ReciprocalYaw_2))
					PitchInput = ReciprocalYaw_1 / ReciprocalYaw_2;
				else if (abs(ReciprocalYaw_2) < abs(ReciprocalYaw_1))
					PitchInput = ReciprocalYaw_2 / ReciprocalYaw_1;
			}
		}
		else
			PitchInput = 1.0;

		if (PitchInput > 1.0 && PitchInput < -1.0)
		{
			// note: sometimes PitchInput is off by about 2.e-16 or so
			// that is to say, PitchInput *= ( 1 + 2.e-16 ); that would fix some of the accuracy beyond 15 decimal places
			// additional note: pretty much every nospread method is roughly accurate to 14 decimal places on average

			PitchCosine = PitchInput;
			PitchSine = sqrt(1.0 - PitchInput * PitchInput);

			Vector vecUnRotatedPitch(PitchSine, PitchCosine, 0.f);

			TempPitch = QAngles.x;

			if (TempPitch < 0.0)
				TempPitch = -TempPitch;

			QTempPitch.y = (45.0 - TempPitch) * 2.0;

			vecRotatedPitch.VectorRotate(vecUnRotatedPitch, QTempPitch);

			if (QAngles.x < 0.f)
				QInputAngles.x = RAD2DEG(atan2(vecRotatedPitch.y, vecRotatedPitch.x));
			else
				QInputAngles.x = RAD2DEG(atan2(-vecRotatedPitch.y, vecRotatedPitch.x));
		}

		QInputAngles.Normalize();
		QInputAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecDirection = vecForward + (vecRight * vecSpread.x) + (vecUp * vecSpread.y);
		vecDirection.Normalize();

		QAdjusterAngles.x = QAngles.x + RAD2DEG(atan2(vecDirection.z, vecDirection.x));
		QAdjusterAngles.Normalize();

		QNewAngles = QOldAngles;
		QNewAngles.x += QAdjusterAngles.x;
		QNewAngles.y += QAdjusterAngles.y;
		QNewAngles.z = 0.f;
		QNewAngles.Normalize();

		// Recover extra precision past 1.e-15, not necessary, but an additional step if you wish, 5 iterations is the maximum you need
		for (size_t i = 0; i < 5; i++)
		{
			QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

			vecDirection = vecForward + (vecRight * vecSpread.x) + (vecUp * vecSpread.y);
			vecDirection.Normalize();

			QTestAngles = vecDirection.ToEulerAngles();
			QTestAngles.Normalize();

			QNewAngles.x += QAngles.x - QTestAngles.x;
			QNewAngles.y += QAngles.y - QTestAngles.y;
			QNewAngles.Normalize();
		}

		if (type == NOSPREAD_PITCH_YAW_ROLL)
		{
			// this only applies in games where you have access to the roll angle, eg the Source Engine, eg CS:S, DoD:S, TF2, CSGO, etc
			if (QInAngles.x > PitchBreakingPoint || QInAngles.x < -PitchBreakingPoint)
			{
				QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				InputRoll = vecUp.y / UpVal;

				QInputRoll.x = RAD2DEG(asin(InputRoll));

				QInputRoll.x *= -1;

				QNewAngles = QOldAngles;

				QNewAngles.z = QInputRoll.x + ConstantRemainder;

				QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				vecDirection = vecForward + (vecRight * -vecSpread.x) + (vecUp * -vecSpread.y);

				vecDirection.Normalize();

				QNewAngles = vecDirection.ToEulerAngles(&vecUp);

				QNewAngles.Normalize();

				QNewAngles.z += ConstantRemainder;

				QNewAngles.Normalize();
			}
		}

		QOutAngles = QNewAngles;
	}*/
}

#pragma warning(default: 4244)