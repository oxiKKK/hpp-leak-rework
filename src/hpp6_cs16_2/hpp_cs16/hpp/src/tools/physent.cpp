#include "framework.h"

physent_t* Physent::GetPhysent(const int& index)
{
	for (int i = 0; i < pmove->numphysent; i++)
	{
		if (pmove->physents[i].info == index)
			return &pmove->physents[i];
	}

	/*// Create new physent
	if (pmove->numphysent + 1 < MAX_PHYSENTS)
	{
		physent_t* physent = &pmove->physents[pmove->numphysent];
		pmove->numphysent++;
		physent->info = index;
		return physent;
	}*/

	return nullptr;
}

void Physent::SetSolid(const int& index, const int& solid)
{
	physent_t *physent = GetPhysent(index);

	if (physent)
		physent->solid = solid;
}

void Physent::SetOrigin(const int& index, const Vector& origin)
{
	physent_t* physent = GetPhysent(index);

	if (physent)
		physent->origin = origin;
}

void Physent::SetMaxs(const int& index, const Vector& maxs)
{
	physent_t* physent = GetPhysent(index);

	if (physent)
		physent->maxs = maxs;
}

void Physent::SetMins(const int& index, const Vector& mins)
{
	physent_t* physent = GetPhysent(index);

	if (physent)
		physent->mins = mins;
}

void Physent::SetAngles(const int& index, const QAngle& angles)
{
	physent_t* physent = GetPhysent(index);

	if (physent)
		physent->angles = angles;
}