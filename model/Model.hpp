// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MODEL_HPP_INCLUDED
#define GAS_MODEL_MODEL_HPP_INCLUDED

#include <cstddef>

//==============================================
// SIMULATION PROPERTIES                        
//==============================================

#define IDEAL     0
#define BOUNCY    1
#define POTENTIAL 2
#define GAS_TYPE POTENTIAL

const size_t MAX_NUMBER_OF_MOLECULES = 10000;
using PhysVal_t = double;

//==============================================

#include "Vector.hpp"
#include "Molecule.hpp"
#include "MoleculeTypes.hpp"
#include "Walls.hpp"

// Barnes-Hut Oct-Tree
struct OctTreeNode
{
	int octs[8];
	int prev;
	int molecule;
	int count;
	Vector center;

	OctTreeNode() = default;

	void initNode(int moleculeI, int prevI, unsigned newCount, Vector newCenter);
};

// Gas Model class
class GasModel 
{
public:
	// Ctor && dtor:
	GasModel(Vector boxSize);
	~GasModel();

	// System properties
	void addMolecule(Molecule mol);

	// Oct-Tree Stuff
	char calculateOct(size_t moleculeI, int curI) const;
	void insertNode(int moleculeI, int prevI, unsigned newCount, unsigned depth, char oct);
	void buildOctTree();

	// Collision:
	void collideOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth);
	void attractOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth);
	void interactWithEachOtherNaive();
	void interactWithEachOther();

	// General simulation cycle:
	void iterationCycle();

	// Box:
	GasContainer box;

	// Molecules:
	Molecule* molecules;
	size_t moleculeCount;

	// Oct-Tree stuff:
	OctTreeNode* octTree;
	size_t octTreeSize;
	bool octTreeFuckedUp;
	Vector* sizeAtDepth;
};

#endif  // GAS_MODEL_MODEL_HPP_INCLUDED
