// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_MODEL_INCLUDED
#define GAS_MODEL_MODEL_INCLUDED

#include <cmath>
#include <limits>
#include <cstdio>

#include "Dimensioning.h"

// SIMULATION PROPERTIES
const size_t MAX_NUMBER_OF_MOLECULES = 100000;
const PhysVal_t RADIUS = 10.0;

// GAS TYPES
#define IDEAL  0
#define BOUNCY 1
#define GAS_TYPE BOUNCY

// CALCULATION PROPERTIES
const size_t OCT_TREE_MAX_NODES = 4 * MAX_NUMBER_OF_MOLECULES;
const size_t OCT_TREE_MAX_SEPARTION_TRIES = ceil(log2(OCT_TREE_MAX_NODES));
const size_t OCT_TREE_MAX_DEPTH = 10 * ceil(log2(OCT_TREE_MAX_NODES));

// 3D-vector with basic arithmetic
struct Vector
{
	PhysVal_t x, y, z;

	Vector operator+(const Vector& vec) const;

	Vector operator-(const Vector& vec) const;

	Vector& operator+=(const Vector& vec);

	Vector& operator-=(const Vector& vec);

	Vector operator*(PhysVal_t k) const;

	Vector& operator*=(PhysVal_t k);

	PhysVal_t lenSqr() const;

	PhysVal_t length() const;

	void setLength(PhysVal_t newLen);

	PhysVal_t scalar(const Vector& v) const;

};


// Barnes-Hut Oct-Tree
struct OctTreeNode
{
	int octs[8];
	int prev;
	int molecule;
	int count;
	Vector centerOfMass;
	Vector center;

	void initNode(int moleculeI, int prevI, unsigned newCount, Vector newCenter);

};

// Model

// GAS MODEL CLASS
class GasModel
{
public:
	// Ctor && dtor:
	GasModel(Vector boxSize);
	~GasModel();

	// System properties
	void addMolecule(Vector coord, Vector speed);

	// Oct-Tree Stuff
	char calculateOct(size_t moleculeI, int curI) const;
	void insertNode(int moleculeI, int prevI, unsigned newCount, unsigned depth, char oct);
	void buildOctTree();

	// Movement:
	void move();

	// Collision:
	void collideWithWalls();
	void collideTwoMolecules(size_t i, size_t j);
	void collideOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth);
	void collideWithEachOtherNaive();
	void collideWithEachOther();

	// Simulation properties:
	Vector boxSize;
	size_t size;

	// Molecules:
	Vector* coords;
	Vector* speeds;

	// Oct-Tree stuff:
	OctTreeNode* octTree;
	size_t octTreeSize;
	bool octTreeFuckedUp;
	Vector* sizeAtDepth;

	// Make it isotropic, please!
	unsigned tick;
};

#endif  // GAS_MODEL_MODEL_HPP_INCLUDED
