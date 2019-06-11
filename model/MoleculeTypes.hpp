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
	SAS_2_Model( 4.00, 0, 0, 1),
	SAS_2_Model(39.95, 0, 0, 1)
};

//==============================================
// COLLISION PROPERTIES
//==============================================

const PhysVal_t COLLISION_RADIUS[TYPES_COUNT] = 
{
	SAS_2_Model(1.28, 0, 1, 0),
	SAS_2_Model(1.91, 0, 1, 0)
};

const PhysVal_t MAXIMUM_COLLISION_RADIUS          = COLLISION_RADIUS[ARGON];
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
	SAS_2_Model(8.45e24, -2, 2, 1), // He He
	SAS_2_Model(2.90e25, -2, 2, 1), // He Ar
	SAS_2_Model(2.90e25, -2, 2, 1), // Ar He
	SAS_2_Model(9.92e25, -2, 2, 1)  // Ar Ar
};

PhysVal_t LennardJonesForceA(MoleculeType typeA, MoleculeType typeB);
PhysVal_t LennardJonesForceB(MoleculeType typeA, MoleculeType typeB);

PhysVal_t LennardJonesPotentialA(MoleculeType typeA, MoleculeType typeB);
PhysVal_t LennardJonesPotentialB(MoleculeType typeA, MoleculeType typeB);

const PhysVal_t LENNARD_JONES_FORCE_A[TYPES_COUNT_SQR] = 
{
	LennardJonesForceA(HELIUM, HELIUM),
	LennardJonesForceA(HELIUM,  ARGON),
	LennardJonesForceA( ARGON, HELIUM),
	LennardJonesForceA( ARGON,  ARGON)
};

const PhysVal_t LENNARD_JONES_FORCE_B[TYPES_COUNT_SQR] = 
{
	LennardJonesForceB(HELIUM, HELIUM),
	LennardJonesForceB(HELIUM,  ARGON),
	LennardJonesForceB( ARGON, HELIUM),
	LennardJonesForceB( ARGON,  ARGON)
};

const PhysVal_t LENNARD_JONES_POTENTIAL_A[TYPES_COUNT_SQR] = 
{
	LennardJonesPotentialA(HELIUM, HELIUM),
	LennardJonesPotentialA(HELIUM,  ARGON),
	LennardJonesPotentialA( ARGON, HELIUM),
	LennardJonesPotentialA( ARGON,  ARGON)
};

const PhysVal_t LENNARD_JONES_POTENTIAL_B[TYPES_COUNT_SQR] = 
{
	LennardJonesPotentialB(HELIUM, HELIUM),
	LennardJonesPotentialB(HELIUM,  ARGON),
	LennardJonesPotentialB( ARGON, HELIUM),
	LennardJonesPotentialB( ARGON,  ARGON)
};

const PhysVal_t POTENTIAL_CUTOFF_MAX_RADIUS          = 7 * MAXIMUM_COLLISION_RADIUS;
const PhysVal_t POTENTIAL_CUTOFF_MAX_RADIUS_SQUAREx4 = 4 * std::pow(POTENTIAL_CUTOFF_MAX_RADIUS, 2);

PhysVal_t LennardJonesForce    (MoleculeType typeA, MoleculeType typeB, PhysVal_t distance);
PhysVal_t LennardJonesPotential(MoleculeType typeA, MoleculeType typeB, PhysVal_t distance);


#endif // GAS_MODEL_MOLECULE_TYPES_HPP_INCLUDED
