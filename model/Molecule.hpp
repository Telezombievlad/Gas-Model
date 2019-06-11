// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MOLECULE_HPP_INCLUDED
#define GAS_MODEL_MOLECULE_HPP_INCLUDED

#include "MoleculeTypes.hpp"
#include "Vector.hpp"

struct Molecule
{
public:
	Vector coords;
	Vector speed;
	Vector force;
	MoleculeType type;

	Molecule() = default;

	Molecule(Vector newCoords, Vector newSpeed, MoleculeType newType);

	inline void integrationStep();
};

void moleculesCollide(Molecule& molA, Molecule& molB);

void moleculesAttract(PhysVal_t& potEnergy, Molecule& molA, Molecule& molB);

#endif // GAS_MODEL_MOLECULE_HPP_INCLUDED