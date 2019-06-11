// No Copyright. Vladislav Aleinik 2019
#include "Dimensioning.hpp"

#include <cmath>

PhysVal_t SAS_2_Model(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( TIME_DELTA, -dimTime ) *
	               std::pow(SPACE_DELTA, -dimSpace) *
	               std::pow( MASS_DELTA, -dimMass );
}

PhysVal_t Model_2_SAS(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( TIME_DELTA, dimTime ) *
	               std::pow(SPACE_DELTA, dimSpace) *
	               std::pow( MASS_DELTA, dimMass );
}
