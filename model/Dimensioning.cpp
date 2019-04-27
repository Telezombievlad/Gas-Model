#include "Dimensioning.h"

constexpr PhysVal_t SI_2_Model(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( TIME_DELTA, -dimTime ) *
	               std::pow(SPACE_DELTA, -dimSpace) *
	               std::pow( MASS_DELTA, -dimMass );
}


constexpr PhysVal_t Model_2_SI(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( TIME_DELTA, dimTime ) *
	               std::pow(SPACE_DELTA, dimSpace) *
	               std::pow( MASS_DELTA, dimMass );
}
