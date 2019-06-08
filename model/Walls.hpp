// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_WALLS_HPP_INCLUDED
#define GAS_MODEL_WALLS_HPP_INCLUDED

#include "Molecule.hpp"

class GasContainer
{
public:
	Vector containerSize;

	GasContainer(Vector boxSize);

	void moleculeBounce(Molecule& mol);
};

#endif  // GAS_MODEL_WALLS_HPP_INCLUDED