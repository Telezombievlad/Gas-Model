#include "Model.h"

///// Vector functions

Vector Vector::operator+(const Vector& vec) const
{
  return {x + vec.x, y + vec.y, z + vec.z};
}

Vector Vector::operator-(const Vector& vec) const
{
	return {x - vec.x, y - vec.y, z - vec.z};
}

Vector& Vector::operator+=(const Vector& vec)
{
  x += vec.x;
  y += vec.y;
  z += vec.z;

  return *this;
}

Vector& Vector::operator-=(const Vector& vec)
{
  x -= vec.x;
  y -= vec.y;
  z -= vec.z;

  return *this;
}

Vector Vector::operator*(PhysVal_t k) const
{
	return {k*x, k*y, k*z};
}

Vector& Vector::operator*=(PhysVal_t k)
{
  x *= k;
  y *= k;
  z *= k;

  return *this;
}

PhysVal_t Vector::lenSqr() const
{
	return x*x + y*y + z*z;
}

PhysVal_t Vector::length() const
{
  return std::sqrt(x*x + y*y + z*z);
}

void Vector::setLength(PhysVal_t newLen)
{
	PhysVal_t curLen = length();

	if (std::abs(curLen) < 10 * std::numeric_limits<PhysVal_t>::epsilon()) return;

	operator*=(newLen / curLen);
}

PhysVal_t Vector::scalar(const Vector& v) const
{
	return v.x*x + v.y*y + v.z*z;
}

///// Barnes-Hut

void OctTreeNode::initNode(int moleculeI, int prevI, unsigned newCount, Vector newCenter)
{
  for (size_t i = 0; i < 8; ++i) octs[i] = -1;
  prev = prevI;
  molecule = moleculeI;
  count = newCount;
  centerOfMass = {0, 0, 0};
  center = newCenter;
}

///// Model

GasModel::GasModel(Vector newBoxSize) :
	boxSize (newBoxSize),
	size    (0),
	coords  (new Vector[MAX_NUMBER_OF_MOLECULES]),
	speeds  (new Vector[MAX_NUMBER_OF_MOLECULES]),
	octTree (new OctTreeNode[OCT_TREE_MAX_NODES]),
	octTreeSize     (0),
	octTreeFuckedUp (false),
	sizeAtDepth (new Vector[OCT_TREE_MAX_DEPTH]),
	tick (0)
{
	if (!coords || !speeds || !octTree || !sizeAtDepth)
	{
		printf("GasModel::ctor(): Unable to allocate memory!\n");
		exit(1);
	}

	for (size_t i = 0; i < OCT_TREE_MAX_DEPTH; ++i)
	{
		sizeAtDepth[i] = boxSize * std::pow(0.5, i + 1);
	}
}

GasModel::~GasModel()
{
	delete[] coords;
	delete[] speeds;
	delete[] octTree;
	delete[] sizeAtDepth;
}

void GasModel::addMolecule(Vector coord, Vector speed)
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

	tick = (tick + 1) % 2;
}

char GasModel::calculateOct(size_t moleculeI, int curI) const
{
	return ((coords[moleculeI].x > octTree[curI].center.x) ? 4 : 0) +
	       ((coords[moleculeI].y > octTree[curI].center.y) ? 2 : 0) +
	       ((coords[moleculeI].z > octTree[curI].center.z) ? 1 : 0);
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
	if (size == 0) return;

	octTree[0].initNode(0, -1, 1, sizeAtDepth[0]);
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

void GasModel::collideWithWalls()
{
	for (size_t mol = 0; mol < size; ++mol)
	{
		Vector cur = coords[mol];

		if (cur.x < RADIUS)
		{
			cur.x = 2 * RADIUS - cur.x;
			speeds[mol].x *= -1;
		}
		else if (cur.x > boxSize.x - RADIUS)
		{
			cur.x = 2 * (boxSize.x - RADIUS) - cur.x;
			speeds[mol].x *= -1;
		}

		if (cur.y < RADIUS)
		{
			cur.y = 2 * RADIUS - cur.y;
			speeds[mol].y *= -1;
		}
		else if (cur.y > boxSize.y - RADIUS)
		{
			cur.y = 2 * (boxSize.y - RADIUS) - cur.y;
			speeds[mol].y *= -1;
		}

		if (cur.z < RADIUS)
		{
			cur.z = 2 * RADIUS - cur.z;
			speeds[mol].z *= -1;
		}
		else if (cur.z > boxSize.z - RADIUS)
		{
			cur.z = 2 * (boxSize.z - RADIUS) - cur.z;
			speeds[mol].z *= -1;
		}

		coords[mol] = cur;
	}

}

const PhysVal_t RADIUS_SQARE_x4 = 4 * RADIUS * RADIUS;
#if GAS_TYPE == IDEAL_GAS
	void GasModel::collideTwoMolecules(size_t i, size_t j)
	{
		Vector coordDiff = coords[i] - coords[j];

		if (coordDiff.lenSqr() > RADIUS_SQARE_x4) return;

		// Shift out of collision:
		coordDiff.setLength(2 * RADIUS);
		coords[i] = coords[j] + coordDiff;

		Vector centerOfMassSpeedx2 = (speeds[i] * masses[i] + speeds[j] * masses[j]) * (2/(masses[i] + masses[j]));
		speeds[i] = centerOfMassSpeedx2 - speeds[i];
		speeds[j] = centerOfMassSpeedx2 - speeds[j];
	}
#elif GAS_TYPE == BOUNCY
	void GasModel::collideTwoMolecules(size_t i, size_t j)
	{
		Vector coordDiff = coords[i] - coords[j];

		if (coordDiff.lenSqr() > RADIUS_SQARE_x4) return;

		// Shift out of collision:
		coordDiff.setLength(2 * RADIUS);
		coords[i] = coords[j] + coordDiff;

		Vector speedDiffProj = coordDiff * (coordDiff.scalar(speeds[i] - speeds[j]) / RADIUS_SQARE_x4);

		speeds[i] -= speedDiffProj;
		speeds[j] += speedDiffProj;
	}
#endif // IDEAL_GAS


void GasModel::collideOneMoleculeBarnesHut(int moleculeI, int curI, unsigned depth)
{
	if (std::abs(octTree[curI].center.x - coords[moleculeI].x) > sizeAtDepth[depth].x + RADIUS) return;
	if (std::abs(octTree[curI].center.y - coords[moleculeI].y) > sizeAtDepth[depth].y + RADIUS) return;
	if (std::abs(octTree[curI].center.z - coords[moleculeI].z) > sizeAtDepth[depth].z + RADIUS) return;

	if (octTree[curI].count == 1)
	{
		collideTwoMolecules(moleculeI, octTree[curI].molecule);
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

void GasModel::collideWithEachOtherNaive()
{
	if (tick % 2 == 0)
	{
		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				collideTwoMolecules(i, j);
			}
		}
	}
	else
	{
		for (size_t i = 0; i < size; ++i)
		{
			for (size_t j = i + 1; j < size; ++j)
			{
				collideTwoMolecules(j, i);
			}
		}
	}
}

void GasModel::collideWithEachOther()
{
	buildOctTree();

	if (octTreeFuckedUp) collideWithEachOtherNaive();

	if (tick % 2 == 0)
	{
		for (size_t i = 0; i < size; ++i)
			collideOneMoleculeBarnesHut(i, 0, 0);
	}
	else
	{
		for (size_t i = size; 0 < i; --i)
			collideOneMoleculeBarnesHut(i-1, 0, 0);
	}
}
