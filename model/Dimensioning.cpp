#include "Dimensioning.h"

PhysVal_t SI_2_Model(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( 1, dimTime ) * std::pow( TIME_DELTA, -dimTime ) *
	               std::pow( 1e10, dimSpace) * std::pow( SPACE_DELTA, -dimSpace ) *
	               std::pow( 6e26, dimMass ) * std::pow( MASS_DELTA, -dimMass );
}


PhysVal_t Model_2_SI(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass)
{
	return value * std::pow( 1, -dimTime ) * std::pow( TIME_DELTA, dimTime ) *
	               std::pow( 1e10, -dimSpace) * std::pow( SPACE_DELTA, dimSpace ) *
	               std::pow( 6e26, -dimMass ) * std::pow( MASS_DELTA, dimMass );
}
