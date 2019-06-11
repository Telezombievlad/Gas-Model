// No Copyright. Vladislav Aleinik 2019
#include "MoleculeTypes.hpp"

PhysVal_t LennardJonesForceA(MoleculeType typeA, MoleculeType typeB)
{
	return -12 * 4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  12);
}

PhysVal_t LennardJonesForceB(MoleculeType typeA, MoleculeType typeB)
{
	return 6 * 4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  6);
}

PhysVal_t LennardJonesPotentialA(MoleculeType typeA, MoleculeType typeB)
{
	return 4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  12);
}

PhysVal_t LennardJonesPotentialB(MoleculeType typeA, MoleculeType typeB)
{
	return -4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  6);
}

PhysVal_t LennardJonesForce(MoleculeType typeA, MoleculeType typeB, PhysVal_t distance)
{
	// Below 2^(1/6) * <sum of radiuses> force rockets to infinity 
	PhysVal_t cutoffDistance = 1.0001 * (COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB]);
	if (distance < cutoffDistance) return 0.0;

	size_t pairIndex = typeA * TYPES_COUNT + typeB;

	PhysVal_t power1 = 1/distance;
	PhysVal_t power2 = power1*power1;
	PhysVal_t power6 = power2*power2*power2;

	return (LENNARD_JONES_FORCE_A[pairIndex] * power6 + LENNARD_JONES_FORCE_B[pairIndex]) * power6 * power1;
}

PhysVal_t LennardJonesPotential(MoleculeType typeA, MoleculeType typeB, PhysVal_t distance)
{
	// Below 2^(1/6) * <sum of radiuses> force rockets to infinity
	PhysVal_t cutoffDistance = 1.0001 * (COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB]);
	if (distance < cutoffDistance) distance = cutoffDistance;

	size_t pairIndex = typeA * TYPES_COUNT + typeB;

	PhysVal_t power1 = 1/distance;
	PhysVal_t power2 = power1*power1;
	PhysVal_t power6 = power2*power2*power2;

	return (LENNARD_JONES_POTENTIAL_A[pairIndex] * power6 + LENNARD_JONES_POTENTIAL_B[pairIndex]) * power6;
}