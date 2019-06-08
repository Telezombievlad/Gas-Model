// No Copyright. Vladislav Aleinik 2019
#include "Molecule.hpp"

Molecule::Molecule(Vector newCoords, Vector newSpeed, MoleculeType newType) :
	coords       (newCoords),
	speed        (newSpeed),
	acceleration ({0, 0, 0}),
	type         (newType)
{}

inline void Molecule::integrationStep()
{
	coords += speed + acceleration * 0.5;
	speed += acceleration;
	acceleration = {0, 0, 0};
}

void moleculesCollide(Molecule& molA, Molecule& molB)
{
#if GAS_TYPE == IDEAL_GAS

	Vector coordDiff = molA.coords - molB.coords;

	if (coordDiff.lenSqr() > MAXIMUM_COLLISION_RADIUS_SQUAREx4) return;

	// Shift out of collision:
	coordDiff.setLength(COLLISION_RADIUS[molA.type] + COLLISION_RADIUS[molB.type]);
	molA.coords = molB.coords + coordDiff;

	Vector centerOfMassSpeedx2 = /*2*(m1v1+m2v2)/(m1+m2)*/
		(molA.speed * MASSES[molA.type] + molB.speed * MASSES[molB.type]) * (2/(MASSES[molA.type] + MASSES[molB.type]));
	molA.speed = centerOfMassSpeedx2 - molA.speed;
	molB.speed = centerOfMassSpeedx2 - molB.speed;	

#elif GAS_TYPE == BOUNCY || GAS_TYPE == POTENTIAL

	Vector coordDiff = molA.coords - molB.coords;

	if (coordDiff.lenSqr() > MAXIMUM_COLLISION_RADIUS_SQUAREx4) return;

	// Shift out of collision:
	PhysVal_t radiusSum = COLLISION_RADIUS[molA.type] + COLLISION_RADIUS[molB.type];
	coordDiff.setLength(radiusSum);
	molA.coords = molB.coords + coordDiff;

	Vector speedDiffProj = 
		coordDiff * (coordDiff.scalar(molA.speed - molB.speed) / (radiusSum * radiusSum));

	molA.speed -= speedDiffProj;
	molB.speed += speedDiffProj;
	
#else
	static_assert(false, "moleculesCollide: Unknown gas type: GAS_TYPE should be IDEAL_GAS, BOUNCY or POTENTIAL\n");
#endif
}

void moleculesAttract(Molecule& molA, Molecule& molB)
{
#if GAS_TYPE == IDEAL_GAS || GAS_TYPE == BOUNCY

	return;

#elif GAS_TYPE == POTENTIAL

	Vector coordDiff = molA.coords - molB.coords;
	if (coordDiff.lenSqr() > POTENTIAL_CUTOFF_MAX_RADIUS_SQUAREx4) return;

	PhysVal_t forceMod = LennardJonesForce(molA.type, molB.type, coordDiff.length());

	Vector force = coordDiff;
	force.setLength(forceMod);

	molA.acceleration -= force / MASSES[molA.type];
	molB.acceleration += force / MASSES[molB.type];

#else
	static_assert(false, "moleculesInteract: Unknown gas type: GAS_TYPE should be IDEAL_GAS, BOUNCY or POTENTIAL\n");
#endif
}