// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED
#define GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED

#include "Dimensioning.hpp"

//==============================================
// MOLECULE TYPES
//==============================================

constexpr size_t TYPES_COUNT = 2;
constexpr size_t TYPES_COUNT_SQR = TYPES_COUNT*TYPES_COUNT;
enum MoleculeType
{
	HELIUM = 0,
	ARGON  = 1
};

//==============================================
// MASSES
//==============================================

const PhysVal_t MASSES[TYPES_COUNT] =
{
	SI_2_Model(2.0, 0, 0, 1),
	SI_2_Model(1.0, 0, 0, 1)
};

//==============================================
// COLLISION PROPERTIES
//==============================================

const PhysVal_t COLLISION_RADIUS[TYPES_COUNT] = 
{
	SI_2_Model(20.0, 0, 1, 0),
	SI_2_Model(10.0, 0, 1, 0)
};

const PhysVal_t MAXIMUM_COLLISION_RADIUS          = COLLISION_RADIUS[HELIUM];
const PhysVal_t MAXIMUM_COLLISION_RADIUS_SQUAREx4 = 4 * std::pow(MAXIMUM_COLLISION_RADIUS, 2);

//==============================================
// LENNARD_JONES INTERACTION PROPERTIES
//==============================================
// U = 4*E*((R/r)^12-(R/r)^6)
// F = A/r^13 + B/r^7
// A = -12*4*E*R^12
// B =   6*4*E*R^6
//==============================================

const PhysVal_t BOND_ENERGY[TYPES_COUNT_SQR] = 
{
	SI_2_Model(1.00, -2, 2, 1), // He He
	SI_2_Model(0.05, -2, 2, 1), // He Ar
	SI_2_Model(0.05, -2, 2, 1), // Ar He
	SI_2_Model(0.05, -2, 2, 1)  // Ar Ar
};

PhysVal_t LennardJonesA(MoleculeType typeA, MoleculeType typeB)
{
	return -12 * 4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  12);
}

PhysVal_t LennardJonesB(MoleculeType typeA, MoleculeType typeB)
{
	return 6 * 4 * BOND_ENERGY[typeA * TYPES_COUNT + typeB] * 
	       std::pow(COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB],  6);
}

const PhysVal_t LENNARD_JONES_A[TYPES_COUNT_SQR] = 
{
	LennardJonesA(HELIUM, HELIUM),
	LennardJonesA(HELIUM,  ARGON),
	LennardJonesA( ARGON, HELIUM),
	LennardJonesA( ARGON,  ARGON)
};

const PhysVal_t LENNARD_JONES_B[TYPES_COUNT_SQR] = 
{
	LennardJonesB(HELIUM, HELIUM),
	LennardJonesB(HELIUM,  ARGON),
	LennardJonesB( ARGON, HELIUM),
	LennardJonesB( ARGON,  ARGON)
};

const PhysVal_t POTENTIAL_CUTOFF_MAX_RADIUS          = 4 * MAXIMUM_COLLISION_RADIUS;
const PhysVal_t POTENTIAL_CUTOFF_MAX_RADIUS_SQUAREx4 = 4 * std::pow(POTENTIAL_CUTOFF_MAX_RADIUS, 2);

PhysVal_t LennardJonesForce(MoleculeType typeA, MoleculeType typeB, PhysVal_t distance)
{
	// Below 2^(1/6) * <sum of radiuses> force rockets to infinity 
	PhysVal_t cutoffDistance = 1.12 * (COLLISION_RADIUS[typeA] + COLLISION_RADIUS[typeB]);
	if (distance < cutoffDistance) distance = cutoffDistance;

	size_t pairIndex = typeA * TYPES_COUNT + typeB;

	PhysVal_t power1 = 1/distance;
	PhysVal_t power2 = power1*power1;
	PhysVal_t power6 = power2*power2*power2;

	return (LENNARD_JONES_A[pairIndex] * power6 + LENNARD_JONES_B[pairIndex]) * power6 * power1;
}

#endif // GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED
