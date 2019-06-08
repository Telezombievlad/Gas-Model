// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED
#define GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED

#include "Dimensioning.hpp"

#include <cmath>

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
	SAS_2_Model(2.0, 0, 0, 1),
	SAS_2_Model(1.0, 0, 0, 1)
};

//==============================================
// COLLISION PROPERTIES
//==============================================

const PhysVal_t COLLISION_RADIUS[TYPES_COUNT] = 
{
	SAS_2_Model(20.0, 0, 1, 0),
	SAS_2_Model(10.0, 0, 1, 0)
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
	SAS_2_Model(1.00, -2, 2, 1), // He He
	SAS_2_Model(0.05, -2, 2, 1), // He Ar
	SAS_2_Model(0.05, -2, 2, 1), // Ar He
	SAS_2_Model(0.05, -2, 2, 1)  // Ar Ar
};

PhysVal_t LennardJonesA(MoleculeType typeA, MoleculeType typeB);
PhysVal_t LennardJonesB(MoleculeType typeA, MoleculeType typeB);

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

PhysVal_t LennardJonesForce(MoleculeType typeA, MoleculeType typeB, PhysVal_t distance);

#endif // GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED
