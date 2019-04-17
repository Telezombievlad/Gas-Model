// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_HPP_INCLUDED
#define GAS_MODEL_HPP_INCLUDED

#include <cmath>
#include <limits>

// ToDo:
// [X] Barnes-Hut OctTree centers of mass fillup
// [X] Barnes-Hut Collisions
// [X] Writing coords to .npy file
// [X] Spinning degrees of freedom

using PhysVal_t = double;

// ASSUMED MODELLING PROPERTIES (DIVIDE BY THEM TO GET REAL VALUE)
const PhysVal_t TIME_DELTA  = 1.0;
const PhysVal_t SPACE_DELTA = 1.0;
const PhysVal_t MASS_DELTA  = 1.0;

// Simulation properties
const size_t MAX_NUMBER_OF_MOLECULES = 10000;
const size_t OCT_TREE_MAX_NODES = 3 * MAX_NUMBER_OF_MOLECULES;
const size_t OCT_TREE_MAX_SEPARTION_TRIES = 7;

const PhysVal_t RADIUS = 10.0;

// 3D-vector with basic arithmetic
struct Vector
{
	PhysVal_t x, y, z;

	inline Vector operator+(const Vector& vec) const
	{
		return {x + vec.x, y + vec.y, z + vec.z};
	}

	inline Vector operator-(const Vector& vec) const 
	{
		return {x - vec.x, y - vec.y, z - vec.z};
	}

	inline Vector& operator+=(const Vector& vec)
	{
		x += vec.x;
		y += vec.y;
		z += vec.z;

		return *this;
	}

	inline Vector& operator-=(const Vector& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;

		return *this;
	}
	
	inline Vector operator*(PhysVal_t k) const
	{
		return {k*x, k*y, k*z};
	}

	inline Vector& operator*=(PhysVal_t k)
	{
		x *= k;
		y *= k;
		z *= k;

		return *this;
	}

	inline PhysVal_t lenSqr() const
	{
		return x*x + y*y + z*z;
	}

	inline PhysVal_t length() const
	{
		return std::sqrt(x*x + y*y + z*z);
	}

	inline void setLength(PhysVal_t newLen)
	{
		PhysVal_t curLen = length();

		if (std::abs(curLen) < 10 * std::numeric_limits<PhysVal_t>::epsilon()) return; 

		operator*=(newLen / curLen);
	}

	inline PhysVal_t scalar(const Vector& v) const
	{
		return v.x*x + v.y*y + v.z*z;
	}
};

// Barnes-Hut Oct-Tree
struct OctTreeNode
{
	int octs[8];
	int prev;
	int molecule;
	int count;
	Vector centerOfMass;
	PhysVal_t centerX;
	PhysVal_t centerY;
	PhysVal_t centerZ;

	void initNode(int moleculeI, int prevI, unsigned newCount, PhysVal_t newCenterX, PhysVal_t newCenterY, PhysVal_t newCenterZ)
	{
		for (size_t i = 0; i < 8; ++i) octs[i] = -1;
		prev = prevI;
		molecule = moleculeI;
		count = newCount;
		centerOfMass = {0, 0, 0};
		centerX = newCenterX;
		centerY = newCenterY;
		centerZ = newCenterZ;	
	}
};

// GAS MODEL CLASS
class GasModel 
{
public:
	GasModel(PhysVal_t newMaxX, PhysVal_t newMaxY, PhysVal_t newMaxZ);

	void addMolecule(Vector coord, Vector speed, PhysVal_t radius, PhysVal_t mass);
	void move();

	void buildOctTree();

	void collideWithWalls(); 
	void collideWithEachOther();

	// Variables:
	PhysVal_t maxX, maxY, maxZ;

	size_t size;

	Vector coords[MAX_NUMBER_OF_MOLECULES];
	Vector speeds[MAX_NUMBER_OF_MOLECULES];

private:
	// Oct-tree
	OctTreeNode octTree[OCT_TREE_MAX_NODES]; 
	size_t octTreeSize;
	bool octTreeFuckedUp; 
 
	char calculateOct(size_t moleculeI, int curI) const;
	void insertNode(int moleculeI, int prevI, unsigned newCount, unsigned depth, char oct);
};

GasModel::GasModel(PhysVal_t newMaxX, PhysVal_t newMaxY, PhysVal_t newMaxZ) :
	maxX (newMaxX),
	maxY (newMaxY),
	maxZ (newMaxZ),
	size (0),
	coords (), speeds (), octTree (),
	octTreeSize (0),
	octTreeFuckedUp (false)
{}

void GasModel::addMolecule(Vector coord, Vector speed, PhysVal_t radius, PhysVal_t mass)
{
	if (size == MAX_NUMBER_OF_MOLECULES) return;

	coords[size] = coord;
	speeds[size] = speed;

	++size;
}

void GasModel::move()
{
	for (size_t i = 0; i < size; ++i)
	{
		coords[i] += speeds[i];
	}
}


inline char GasModel::calculateOct(size_t moleculeI, int curI) const
{
	return (coords[moleculeI].x > octTree[curI].centerX ? 4 : 0) +
	       (coords[moleculeI].y > octTree[curI].centerY ? 2 : 0) + 
	       (coords[moleculeI].z > octTree[curI].centerZ ? 1 : 0);
}

inline void GasModel::insertNode(int moleculeI, int prevI, unsigned newCount, unsigned depth, char oct)
{
	if (octTreeSize == OCT_TREE_MAX_NODES)
	{
		octTreeFuckedUp = true;
		return;
	}

	octTree[prevI].octs[0 + oct] = octTreeSize;

	PhysVal_t depthCoeff = pow(0.5, depth + 1);
	PhysVal_t curCenterX = octTree[prevI].centerX + depthCoeff * ((oct & 0b00000100)? maxX : -maxX);
	PhysVal_t curCenterY = octTree[prevI].centerY + depthCoeff * ((oct & 0b00000010)? maxY : -maxY);
	PhysVal_t curCenterZ = octTree[prevI].centerZ + depthCoeff * ((oct & 0b00000001)? maxZ : -maxZ);

	octTree[octTreeSize].initNode(moleculeI, prevI, newCount, curCenterX, curCenterY, curCenterZ);

	++octTreeSize;
}

void GasModel::buildOctTree()
{
	if (size == 0) return;

	octTree[0].initNode(0, -1, 1, 0.5 * maxX, 0.5 * maxY, 0.5 * maxZ);
	octTreeSize = 1;
	octTreeFuckedUp = false;

	for (size_t moleculeI = 1; moleculeI < size; ++moleculeI)
	{
		// Search for place to insert node in an oct-tree
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

void GasModel::collideWithWalls()
{

	for (size_t i = 0; i < size; ++i)
	{
		Vector cur = coords[i];

		if (cur.x < RADIUS)
		{
			cur.x = 2 * RADIUS - cur.x;
			speeds[i].x *= -1;
		}
		else if (cur.x > maxX - RADIUS)
		{
			cur.x = 2 * (maxX - RADIUS) - cur.x;
			speeds[i].x *= -1;
		}

		if (cur.y < RADIUS)
		{
			cur.y = 2 * RADIUS - cur.y;
			speeds[i].y *= -1;
		}
		else if (cur.y > maxY - RADIUS)
		{
			cur.y = 2 * (maxY - RADIUS) - cur.y;
			speeds[i].y *= -1;
		}

		if (cur.z < RADIUS)
		{
			cur.z = 2 * RADIUS - cur.z;
			speeds[i].z *= -1;
		}
		else if (cur.z > maxZ - RADIUS)
		{
			cur.z = 2 * (maxZ - RADIUS) - cur.z;
			speeds[i].z *= -1;
		}

		coords[i] = cur;
	}
}

const PhysVal_t RADIUS_SQARE_x4 = 4 * RADIUS * RADIUS;
void GasModel::collideWithEachOther()
{
	for (size_t i = 0; i < size; ++i)
	{
		for (size_t j = i + 1; j < size; ++j)
		{
			Vector coordDiff = coords[i] - coords[j];

			if (coordDiff.lenSqr() > RADIUS_SQARE_x4) continue;

			// Shift out of collision:
			coordDiff.setLength(2 * RADIUS);
			coords[i] = coords[j] + coordDiff;

			Vector speedDiffProj = coordDiff * (coordDiff.scalar(speeds[i] - speeds[j]) / RADIUS_SQARE_x4);

			speeds[i] -= speedDiffProj;
			speeds[j] += speedDiffProj;

			// Vector centerOfMassSpeedx2 = (speeds[i] * masses[i] + speeds[j] * masses[j]) * (2/(masses[i] + masses[j]));
			// speeds[i] = centerOfMassSpeedx2 - speeds[i];
			// speeds[j] = centerOfMassSpeedx2 - speeds[j];	
		}
	}
}

#endif  // GAS_MODEL_HPP_INCLUDED

