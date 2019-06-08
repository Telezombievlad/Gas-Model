// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_DIMENSIONING_HPP_INCLUDED
#define GAS_MODEL_DIMENSIONING_HPP_INCLUDED

using PhysVal_t = double;

constexpr PhysVal_t  TIME_DELTA = 1e-13; // Seconds
constexpr PhysVal_t SPACE_DELTA = 0.74;  // Angstrems
constexpr PhysVal_t  MASS_DELTA = 1.008; // Standard Atomic Masses

PhysVal_t SAS_2_Model(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass);
PhysVal_t Model_2_SAS(PhysVal_t value, PhysVal_t dimTime, PhysVal_t dimSpace, PhysVal_t dimMass);

#endif // GAS_MODEL_DIMENSIONING_HPP_INCLUDED
