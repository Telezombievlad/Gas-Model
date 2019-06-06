// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MODEL_HPP_INCLUDED
#define GAS_MODEL_MODEL_HPP_INCLUDED

#include <cmath>
#include <limits>
#include <cstdio>
#include <immintrin.h>

// GAS TYPES
#define IDEAL     0
#define BOUNCY    1
#define POTENTIAL 2
#define GAS_TYPE POTENTIAL

#include "Dimensioning.hpp"
#include "MoleculeTypes.hpp"
#include "Molecule.hpp"
#include "Walls.hpp"

// ToDo:
// [+] Various masses
// [+] Various radiuses
// [+] Potential Branes-Hut
// [+] AVX inside vector operations
// [?] Forces and accelerations

// SIMULATION PROPERTIES
const size_t MAX_NUMBER_OF_MOLECULES = 10000;

// CALCULATION PROPERTIES
const size_t OCT_TREE_MAX_NODES = 4 * MAX_NUMBER_OF_MOLECULES;
const size_t OCT_TREE_MAX_SEPARTION_TRIES = ceil(log2(OCT_TREE_MAX_NODES));
const size_t OCT_TREE_MAX_DEPTH = 10 * ceil(log2(OCT_TREE_MAX_NODES));

// Barnes-Hut Oct-Tree
struct OctTreeNode
{
	int octs[8];
	int prev;
	int molecule;
	int count;
	Vector center;

	OctTreeNode() = default;

	void initNode(int moleculeI, int prevI, unsigned newCount, Vector newCenter)
	{
		for (size_t i = 0; i < 8; ++i) octs[i] = -1;
		prev = prevI;
		molecule = moleculeI;
		count = newCount;
		center = newCenter;
	}
};

//==============================================
// GAS MODEL
//==============================================

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

GasModel::GasModel(Vector newBoxSize) :
	box             (GasContainer(newBoxSize)),
	molecules       (new Molecule[MAX_NUMBER_OF_MOLECULES]),
	moleculeCount   (0),
	octTree         (new OctTreeNode[OCT_TREE_MAX_NODES]),
	octTreeSize     (0),
	octTreeFuckedUp (false),
	sizeAtDepth     (new Vector[OCT_TREE_MAX_DEPTH])
{
	if (!molecules || !octTree || !sizeAtDepth)
	{
		printf("GasModel::ctor(): Unable to allocate memory!\n");
		exit(1);
	}

	for (size_t i = 0; i < OCT_TREE_MAX_DEPTH; ++i)
	{
		sizeAtDepth[i] = box.containerSize * std::pow(0.5, i + 1);
	}
}

GasModel::~GasModel()
{
	delete[] molecules;
	delete[] octTree;
	delete[] sizeAtDepth;
}

//==============================================
// BARNES-HUT TREE CONSTRUCTION
//==============================================

void GasModel::addMolecule(Molecule mol)
{
	if (moleculeCount == MAX_NUMBER_OF_MOLECULES) return;

	molecules[moleculeCount] = mol;

	++moleculeCount;
}

inline char GasModel::calculateOct(size_t moleculeI, int curI) const
{
	return ((molecules[moleculeI].coords.x > octTree[curI].center.x) ? 4 : 0) +
	       ((molecules[moleculeI].coords.y > octTree[curI].center.y) ? 2 : 0) + 
	       ((molecules[moleculeI].coords.z > octTree[curI].center.z) ? 1 : 0);
}

void GasModel::insertNode(int moleculeI, int prevI, unsigned newCount, unsigned depth, char oct)
{
	if (octTreeSize >= OCT_TREE_MAX_NODES || depth >= OCT_TREE_MAX_DEPTH)
	{
		octTreeFuckedUp = true;
		return;
	}

	octTree[prevI].octs[0 + oct] = octTreeSize;

	Vector curCenter = {sizeAtDepth[depth].x * ((oct & 0b00000100)? 1.0 : -1.0),
	                    sizeAtDepth[depth].y * ((oct & 0b00000010)? 1.0 : -1.0),
	                    sizeAtDepth[depth].z * ((oct & 0b00000001)? 1.0 : -1.0)};

	curCenter += octTree[prevI].center;	

	octTree[octTreeSize].initNode(moleculeI, prevI, newCount, curCenter);

	++octTreeSize;
}

void GasModel::buildOctTree()
{
	if (moleculeCount == 0) return;

	octTree[0].initNode(0, -1, 1, sizeAtDepth[0]);
	octTreeSize = 1;
	octTreeFuckedUp = false;

	for (size_t moleculeI = 1; moleculeI < moleculeCount; ++moleculeI)
	{
		// Search for place to insert node into an octo-tree
		int prevI = 0;
		char oct = 0;
		unsigned depth = 0;
		for (int curI = 0; curI != -1;)
		{
			++depth;
			octTree[curI].count += 1;

			oct = calculateOct(moleculeI, curI);

			prevI = curI;
			curI = octTree[curI].octs[0 + oct];
		}

		// Insert node
		if (octTree[prevI].count == 2) // Tree rebalancing needed  
		{
			int oldMoleculeI = octTree[prevI].molecule;
			octTree[prevI].molecule = -1;

			char oldOct = calculateOct(oldMoleculeI, prevI);

			for (unsigned char seperationTries = 0; oct == oldOct && seperationTries < OCT_TREE_MAX_SEPARTION_TRIES; ++seperationTries)
			{
				prevI = octTreeSize;

				insertNode(-1, prevI, 2, depth, oct);
				if (octTreeFuckedUp) return;

				oct    = calculateOct(   moleculeI, prevI);
				oldOct = calculateOct(oldMoleculeI, prevI);

				++depth;
			}

			if (oct != oldOct)
			{
				insertNode(   moleculeI, prevI, 1, depth, oct);
				insertNode(oldMoleculeI, prevI, 1, depth, oldOct);
			}
			else
			{
				insertNode(   moleculeI, prevI, 1, depth, oct);
				insertNode(oldMoleculeI, prevI, 1, depth, oct ^ 1);
			}
		}
		else // Just a single insertion will do
		{
			insertNode(moleculeI, prevI, 1, depth, oct);
		}
	}
}

//==============================================
// MOLECULE INTERACTION
//==============================================

const Vector MAX_COLLISON_BOX_SIZE = Vector(MAXIMUM_COLLISION_RADIUS,
                                            MAXIMUM_COLLISION_RADIUS,
                                            MAXIMUM_COLLISION_RADIUS);

void GasModel::collideOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth)
{
	if (!(octTree[curI].center - molecules[moleculeI].coords).isInBox(sizeAtDepth[depth] + MAX_COLLISON_BOX_SIZE)) return;

	if (octTree[curI].count == 1)
	{
		moleculesCollide(molecules[moleculeI], molecules[octTree[curI].molecule]);
	}
	else
	{
		for (size_t oct = 0; oct < 8; ++oct)
		{
			if (octTree[curI].octs[oct] != -1)
				collideOneMoleculeBarnesHut(moleculeI, octTree[curI].octs[oct], depth + 1);
		}
	}
}

const Vector MAX_POTENTIAL_BOX_SIZE = Vector(POTENTIAL_CUTOFF_MAX_RADIUS,
                                             POTENTIAL_CUTOFF_MAX_RADIUS,
                                             POTENTIAL_CUTOFF_MAX_RADIUS);

void GasModel::attractOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth)
{
	if (!(octTree[curI].center - molecules[moleculeI].coords).isInBox(sizeAtDepth[depth] + MAX_POTENTIAL_BOX_SIZE)) return;

	if (octTree[curI].count == 1)
	{
		moleculesAttract(molecules[moleculeI], molecules[octTree[curI].molecule]);
	}
	else
	{
		for (size_t oct = 0; oct < 8; ++oct)
		{
			if (octTree[curI].octs[oct] != -1)
				attractOneMoleculeBarnesHut(moleculeI, octTree[curI].octs[oct], depth + 1);
		}
	}
}

void GasModel::interactWithEachOtherNaive()
{
	for (size_t i = 0; i < moleculeCount; ++i)
	{
		for (size_t j = i + 1; j < moleculeCount; ++j)
		{
			moleculesCollide(molecules[i], molecules[j]);
			moleculesAttract(molecules[i], molecules[j]);
		}
	}
}

void GasModel::interactWithEachOther()
{
	buildOctTree();

	if (octTreeFuckedUp) interactWithEachOtherNaive();
	else
	{
		for (size_t i = 0; i < moleculeCount; ++i)
		{
			collideOneMoleculeBarnesHut(i, 0, 0);
			attractOneMoleculeBarnesHut(i, 0, 0);
		}
	}
}

//==============================================
// INTERACTION CYCLE
//==============================================

void GasModel::iterationCycle()
{
	for (size_t i = 0; i < moleculeCount; ++i)
		molecules[i].integrationStep();

	interactWithEachOther();

	for (size_t i = 0; i < moleculeCount; ++i)
		box.moleculeBounce(molecules[i]);
}

#endif  // GAS_MODEL_MODEL_HPP_INCLUDED

