#include "main.h"

CNoSpread g_NoSpread;

unsigned int glSeed = 0;

unsigned int seed_table[256] = {
	28985U, 27138U, 26457U, 9451U, 17764U, 10909U, 28790U, 8716U, 6361U, 4853U, 17798U, 21977U, 19643U, 20662U, 10834U, 20103,
	27067U, 28634U, 18623U, 25849U, 8576U, 26234U, 23887U, 18228U, 32587U, 4836U, 3306U, 1811U, 3035U, 24559U, 18399U, 315,
	26766U, 907U, 24102U, 12370U, 9674U, 2972U, 10472U, 16492U, 22683U, 11529U, 27968U, 30406U, 13213U, 2319U, 23620U, 16823,
	10013U, 23772U, 21567U, 1251U, 19579U, 20313U, 18241U, 30130U, 8402U, 20807U, 27354U, 7169U, 21211U, 17293U, 5410U, 19223,
	10255U, 22480U, 27388U, 9946U, 15628U, 24389U, 17308U, 2370U, 9530U, 31683U, 25927U, 23567U, 11694U, 26397U, 32602U, 15031,
	18255U, 17582U, 1422U, 28835U, 23607U, 12597U, 20602U, 10138U, 5212U, 1252U, 10074U, 23166U, 19823U, 31667U, 5902U, 24630,
	18948U, 14330U, 14950U, 8939U, 23540U, 21311U, 22428U, 22391U, 3583U, 29004U, 30498U, 18714U, 4278U, 2437U, 22430U, 3439,
	28313U, 23161U, 25396U, 13471U, 19324U, 15287U, 2563U, 18901U, 13103U, 16867U, 9714U, 14322U, 15197U, 26889U, 19372U, 26241,
	31925U, 14640U, 11497U, 8941U, 10056U, 6451U, 28656U, 10737U, 13874U, 17356U, 8281U, 25937U, 1661U, 4850U, 7448U, 12744,
	21826U, 5477U, 10167U, 16705U, 26897U, 8839U, 30947U, 27978U, 27283U, 24685U, 32298U, 3525U, 12398U, 28726U, 9475U, 10208,
	617U, 13467U, 22287U, 2376U, 6097U, 26312U, 2974U, 9114U, 21787U, 28010U, 4725U, 15387U, 3274U, 10762U, 31695U, 17320,
	18324U, 12441U, 16801U, 27376U, 22464U, 7500U, 5666U, 18144U, 15314U, 31914U, 31627U, 6495U, 5226U, 31203U, 2331U, 4668,
	12650U, 18275U, 351U, 7268U, 31319U, 30119U, 7600U, 2905U, 13826U, 11343U, 13053U, 15583U, 30055U, 31093U, 5067U, 761,
	9685U, 11070U, 21369U, 27155U, 3663U, 26542U, 20169U, 12161U, 15411U, 30401U, 7580U, 31784U, 8985U, 29367U, 20989U, 14203,
	29694U, 21167U, 10337U, 1706U, 28578U, 887U, 3373U, 19477U, 14382U, 675U, 7033U, 15111U, 26138U, 12252U, 30996U, 21409,
	25678U, 18555U, 13256U, 23316U, 22407U, 16727U, 991U, 9236U, 5373U, 29402U, 6117U, 15241U, 27715U, 19291U, 19888U, 19847U
};

unsigned int U_Random()
{
	glSeed *= 69069;
	glSeed += seed_table[glSeed & 0xFF] + 1;
	return (glSeed & 0xFFFFFFF);
}

void U_Srand(unsigned int seed)
{
	glSeed = seed_table[seed & 0xFF];
}

int UTIL_SharedRandomLong(unsigned int seed, int low, int high)
{
	unsigned int range = high - low + 1;
	U_Srand((unsigned int)(high + low + seed));
	if (range != 1) {
		int rnum = U_Random();
		int offset = rnum % range;
		return (low + offset);
	}

	return low;
}

float UTIL_SharedRandomFloat(unsigned int seed, float low, float high)
{
	unsigned int range = high - low;
	U_Srand((unsigned int)seed + *(unsigned int *)&low + *(unsigned int *)&high);

	U_Random();
	U_Random();

	if (range)
	{
		int tensixrand = U_Random() & 0xFFFFu;
		float offset = float(tensixrand) / 0x10000u;
		return (low + offset * range);
	}

	return low;
}

void CNoSpread::Run(usercmd_s *cmd)
{
	if (!cvar.ragebot_active)
		return;

	if (!cvar.ragebot_aim_enabled)
		return;

	if (!cvar.ragebot_remove_spread)
		return;

	if(cmd->buttons & IN_ATTACK && g_Weapon.CanAttack())
	{
		QAngle QAngles(cmd->viewangles), QNewAngles;

		GetSpreadOffset(g_Weapon.data.m_iRandomSeed, 1, QAngles, QNewAngles, cvar.ragebot_remove_spread);

		cmd->viewangles = QNewAngles;
	}
}

double CNoSpread::_GetConstantRemainder(Vector a_vecSpread) // aka Trap God Constant
{
	double ConstantRemainder = 0;

	Vector Forward, Right, Up;

	Vector SpreadAt90Pitch(a_vecSpread[1], -a_vecSpread[0], -1);

	QAngle OutputAngles;

	QAngle InputAngles(90, 0, 0);

	InputAngles.AngleVectors(&Forward, &Right, &Up);

	SpreadAt90Pitch.Normalize();

	OutputAngles = SpreadAt90Pitch.ToEulerAngles(&Up);

	OutputAngles.Normalize();

	ConstantRemainder = OutputAngles[1] - OutputAngles[2];

	return ConstantRemainder;
}

void CNoSpread::GetSpreadXY(unsigned int a_iRandomSeed, int a_iFuture, Vector &vecSpread)
{
	vecSpread[0] = UTIL_SharedRandomFloat(a_iRandomSeed + a_iFuture, -0.5, 0.5) + UTIL_SharedRandomFloat(a_iRandomSeed + 1 + a_iFuture, -0.5, 0.5);
	vecSpread[1] = UTIL_SharedRandomFloat(a_iRandomSeed + 2 + a_iFuture, -0.5, 0.5) + UTIL_SharedRandomFloat(a_iRandomSeed + 3 + a_iFuture, -0.5, 0.5);

	vecSpread[0] *= g_Weapon.data.m_flSpread;
	vecSpread[1] *= g_Weapon.data.m_flSpread;
}

void CNoSpread::GetSpreadOffset(unsigned int a_iRandomSeed, int a_iFuture, QAngle a_QInAngles, QAngle &a_QOutAngles, int a_nType)
{
	if (a_nType == 1) 
	{
		QAngle QNewAngles, QAngles, QAdjusterAngles;

		Vector vecDirection, vecForward, vecRight, vecUp, vecSpread, vecView;

		QAngles.Init(a_QInAngles);

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		GetSpreadXY(a_iRandomSeed, a_iFuture, vecSpread);

		vecDirection = vecForward + (vecSpread[0] * vecRight) + (vecSpread[1] * vecUp);

		vecDirection.Normalize();

		vecView = 8912 * vecDirection;

		g_Utils.VectorAngles(vecView, QAdjusterAngles);

		QNewAngles[0] = QAngles[0] + (QAngles[0] - QAdjusterAngles[0]);
		QNewAngles[1] = QAngles[1] + (QAngles[1] - QAdjusterAngles[1]);
		QNewAngles[2] = 0;

		QNewAngles.Normalize();

		a_QOutAngles = QNewAngles;
	}
	else if (a_nType == 3)
	{
		QAngle QAngles, QNewAngles, QTempAngles;

		Vector vecForward, vecRight, vecUp, vecDir, vecRandom;

		GetSpreadXY(a_iRandomSeed, a_iFuture, vecRandom);

		QAngles = a_QInAngles;

		QTempAngles = QAngle();

		QTempAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecDir = vecForward + (vecRight * vecRandom[0]) + (vecUp * vecRandom[1]);

		vecDir.Normalize();

		QNewAngles = vecDir.ToEulerAngles();

		QNewAngles[0] -= QAngles[0];

		QNewAngles.Normalize();

		QNewAngles.AngleVectorsTranspose(&vecForward, &vecRight, &vecUp);

		vecDir = vecForward;

		QNewAngles = vecDir.ToEulerAngles(&vecUp);

		QNewAngles.Normalize();

		QNewAngles[1] += QAngles[1];

		QNewAngles.Normalize();

		a_QOutAngles = QNewAngles;
	}
	else if (a_nType == 4 || a_nType == 2)
	{
		double PitchSine, PitchCosine;
		double CosineInput, CosinePitch, InputRoll, PitchBreakingPoint, PitchInput;
		double ReciprocalYaw_1, ReciprocalYaw_2, ConstantRemainder, UpVal, Yaw_1;

		QAngle QAdjusterAngles, QAngles, QInputAngles, QInputRoll, QNewAngles, QTestAngles, QOldAngles;

		Vector vecRandom, vecSpread, vecDirection, vecForward, vecRight, vecUp;
		Vector vecReciprocal, vecInputRight, vecInputRight2;
		Vector vecUnRotatedPitch, vecRotatedPitch;

		GetSpreadXY(a_iRandomSeed, a_iFuture, vecSpread);

		ConstantRemainder = _GetConstantRemainder(vecSpread);

		QAngles = QAngle();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecDirection = vecForward + (-vecSpread[0] * vecRight) + (-vecSpread[1] * vecUp);

		vecDirection.Normalize();

		PitchBreakingPoint = RAD2DEG(atan2(vecDirection[1], sqrt((1 - (vecDirection[1] * vecDirection[1])))));

		if (PitchBreakingPoint > 180)
		{
			PitchBreakingPoint -= 360;
		}
		else if (PitchBreakingPoint < -180)
		{
			PitchBreakingPoint += 360;
		}

		if (PitchBreakingPoint < 0)
		{
			PitchBreakingPoint = 90 + PitchBreakingPoint;
		}
		else
		{
			PitchBreakingPoint = 90 - PitchBreakingPoint;
		}

		QAngles.Init(a_QInAngles);

		QOldAngles = QAngles;

		QAngles = vecDirection.ToEulerAngles();

		QAngles.Normalize();

		QAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		UpVal = vecUp[2];

		QAngles = QOldAngles;

		CosineInput = QAngles[0] * (IM_PI * 2 / 360);

		CosinePitch = cos(CosineInput);

		if (CosinePitch != 0)
		{
			Yaw_1 = 1 / CosinePitch;
		}
		else
		{
			Yaw_1 = 0;
		}

		Yaw_1 *= vecDirection[1];

		vecInputRight[1] = Yaw_1;

		if (Yaw_1 >= 1 || Yaw_1 <= -1)
		{
			vecInputRight[1] = 1 / Yaw_1;

			vecInputRight[0] = 0;
		}
		else
		{
			vecInputRight[0] = sqrt((1 - (vecInputRight[1] * vecInputRight[1])));
		}

		vecInputRight[2] = 0;

		QAdjusterAngles[1] = (RAD2DEG(atan2(vecInputRight[1], vecInputRight[0])));

		QAdjusterAngles.Normalize();

		vecInputRight2[1] = vecDirection[1];

		if (vecDirection[1] >= 1 || vecDirection[1] <= -1)
		{
			vecInputRight2[1] = 1 / vecDirection[1];

			vecInputRight2[0] = 0;
		}
		else
		{
			vecInputRight2[0] = sqrt((1 - (vecInputRight2[1] * vecInputRight2[1])));
		}

		if (vecInputRight[0] != 0)
		{
			ReciprocalYaw_1 = vecInputRight[1] / vecInputRight[0];
		}
		else
		{
			ReciprocalYaw_1 = 0;
		}

		if (vecInputRight2[0] != 0)
		{
			ReciprocalYaw_2 = vecInputRight2[1] / vecInputRight2[0];
		}
		else
		{
			ReciprocalYaw_2 = 0;
		}

		PitchInput = 0;

		if (QAngles[0] != 0)
		{
			if (ReciprocalYaw_1 != 0 && ReciprocalYaw_2 != 0)
			{
				PitchInput = 1;

				if (abs(ReciprocalYaw_1) < abs(ReciprocalYaw_2))
				{
					PitchInput = ReciprocalYaw_1 / ReciprocalYaw_2;
				}
				else if (abs(ReciprocalYaw_2) < abs(ReciprocalYaw_1))
				{
					PitchInput = ReciprocalYaw_2 / ReciprocalYaw_1;
				}
			}
		}
		else
		{
			PitchInput = 1;
		}

		QInputAngles = QAngle();

		if (PitchInput > 1 && PitchInput < -1)
		{
			// note: sometimes PitchInput is off by about 2.e-16 or so
			// that is to say, PitchInput *= ( 1 + 2.e-16 ); that would fix some of the accuracy beyond 15 decimal places
			// additional note: pretty much every nospread method is roughly accurate to 14 decimal places on average

			PitchCosine = PitchInput;
			PitchSine = sqrt(1 - PitchInput * PitchInput);

			vecUnRotatedPitch[0] = PitchSine;
			vecUnRotatedPitch[1] = PitchCosine;
			vecUnRotatedPitch[2] = 0;

			double TempPitch = QAngles[0];

			if (TempPitch < 0)
			{
				TempPitch = -TempPitch;
			}

			TempPitch = ((45 - TempPitch) * 2);

			QAngle QTempPitch = QAngle(0, TempPitch, 0);

			vecRotatedPitch.VectorRotate(vecUnRotatedPitch, QTempPitch);

			if (QAngles[0] < 0)
			{
				QInputAngles[0] = RAD2DEG(atan2(vecRotatedPitch[1], vecRotatedPitch[0]));
			}
			else
			{
				QInputAngles[0] = RAD2DEG(atan2(-vecRotatedPitch[1], vecRotatedPitch[0]));
			}
		}

		QInputAngles.Normalize();

		QInputAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

		vecDirection = vecForward + (vecSpread[0] * vecRight) + (vecSpread[1] * vecUp);

		vecDirection.Normalize();

		QAdjusterAngles[0] = QAngles[0] + (RAD2DEG(atan2(vecDirection[2], vecDirection[0])));

		QAdjusterAngles.Normalize();

		QNewAngles = QOldAngles;

		QNewAngles[0] += QAdjusterAngles[0];
		QNewAngles[1] += QAdjusterAngles[1];
		QNewAngles[2] = 0;

		QNewAngles.Normalize();

		//Recover extra precision past 1.e-15, not necessary, but an additional step if you wish, 5 iterations is the maximum you need
		for (int Iterator = 0; Iterator < 5; Iterator++)
		{
			QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

			vecDirection = vecForward + (vecSpread[0] * vecRight) + (vecSpread[1] * vecUp);

			vecDirection.Normalize();

			QTestAngles = vecDirection.ToEulerAngles();

			QTestAngles.Normalize();

			QNewAngles[0] += (QAngles[0] - QTestAngles[0]);
			QNewAngles[1] += (QAngles[1] - QTestAngles[1]);

			QNewAngles.Normalize();
		}

		// this only applies in games where you have access to the roll angle, eg the Source Engine, eg CS:S, DoD:S, TF2, CSGO, etc
		if (a_nType == 4)
		{
			if (a_QInAngles[0] > PitchBreakingPoint || a_QInAngles[0] < -PitchBreakingPoint)
			{
				QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				InputRoll = vecUp[1] / UpVal;

				QInputRoll[0] = RAD2DEG(asin(InputRoll));

				QInputRoll[0] *= -1;

				QNewAngles = QOldAngles;

				QNewAngles[2] = QInputRoll[0] + ConstantRemainder;

				QNewAngles.AngleVectors(&vecForward, &vecRight, &vecUp);

				vecDirection = vecForward + (-vecSpread[0] * vecRight) + (-vecSpread[1] * vecUp);

				vecDirection.Normalize();

				QNewAngles = vecDirection.ToEulerAngles(&vecUp);

				QNewAngles.Normalize();

				QNewAngles[2] += ConstantRemainder;

				QNewAngles.Normalize();
			}
		}

		a_QOutAngles = QNewAngles;
	}
}