// No Copyright. Vladislav Aleinik 2019
#include "Molecule.hpp"

Molecule::Molecule(Vector newCoords, Vector newSpeed, MoleculeType newType) :  
	coords (newCoords),
	speed  (newSpeed),
	force  ({0, 0, 0}),
	type   (newType)
{}

inline void Molecule::integrationStep()
{
#if defined(IDEAL) || defined(BOUNCY)

	coords += speed;

#elif defined(POTENTIAL)

	coords += speed + force / (2 * MASSES[type]);
	speed += force/MASSES[type];
	force = {0, 0, -GRAVITY*MASSES[type]};

#else
	static_assert(false, "integrationStep: Unknown gas type: GAS_TYPE should be IDEAL, BOUNCY or POTENTIAL\n");
#endif
}

void moleculesCollide(Molecule& molA, Molecule& molB)
{
#if defined(IDEAL)

	return;

#elif defined(BOUNCY)

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
	
#elif defined(POTENTIAL)

	Vector coordDiff = molA.coords - molB.coords; 

	if (coordDiff.lenSqr() > MAXIMUM_COLLISION_RADIUS_SQUAREx4) return;

	// Shift out of collision:
	PhysVal_t radiusSum = COLLISION_RADIUS[molA.type] + COLLISION_RADIUS[molB.type];
	coordDiff.setLength(radiusSum);
	molA.coords = (molA.coords + molB.coords + coordDiff)/2;
	molB.coords = molA.coords - coordDiff;

	Vector speedDiffProj = 
		coordDiff * (coordDiff.scalar(molA.speed - molB.speed) / (radiusSum * radiusSum));

	molA.speed -= speedDiffProj;
	molB.speed += speedDiffProj;

#else
	static_assert(false, "moleculesCollide: Unknown gas type: GAS_TYPE should be IDEAL, BOUNCY or POTENTIAL\n");
#endif
}

void moleculesAttract(PhysVal_t& potEnergy, Molecule& molA, Molecule& molB)
{
#if defined(IDEAL) || defined(BOUNCY) 

	return;

#elif defined(POTENTIAL)

	Vector coordDiff = molA.coords - molB.coords;
	if (coordDiff.lenSqr() > POTENTIAL_CUTOFF_MAX_RADIUS_SQUAREx4) return;

	Vector force = coordDiff;
	force.setLength(LennardJonesForce(molA.type, molB.type, coordDiff.length()));

	molA.force -= force;
	molB.force += force;

	potEnergy += LennardJonesPotential(molA.type, molB.type, coordDiff.length());

#else
	static_assert(false, "moleculesInteract: Unknown gas type: GAS_TYPE should be IDEAL, BOUNCY or POTENTIAL\n");
#endif
}