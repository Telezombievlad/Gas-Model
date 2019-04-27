// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_DIMENSIONING_INCLUDED
#define GAS_MODEL_DIMENSIONING_INCLUDED

#include <ratio>
#include <cmath>

// ASSUMED CHARACTERISTIC PROPERTIES
using PhysVal_t = double;

constexpr PhysVal_t TIME_DELTA  = 1.0;   // Seconds;
constexpr PhysVal_t SPACE_DELTA = 0.74;  // Angstrems;
constexpr PhysVal_t MASS_DELTA  = 1.008; // Standard Atomic Masses;

constexpr PhysVal_t SI_2_Model(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass);

constexpr PhysVal_t Model_2_SI(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass);


#endif // GAS_MODEL_DIMENSIONING_HPP_INCLUDED
