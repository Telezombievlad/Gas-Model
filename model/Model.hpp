// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_HPP_INCLUDED
#define GAS_MODEL_HPP_INCLUDED

#include <cmath>
#include <limits>

// ToDo:
// [X] Barnes-Hut OctTree potential
// [X] Grid collision detection

using PhysVal_t = double;

// ASSUMED MODELLING PROPERTIES (DIVIDE BY THEM TO GET REAL VALUE)
const PhysVal_t TIME_DELTA = 1.0;
const PhysVal_t SPACE_DELTA = 1.0;
const PhysVal_t MASS_DELTA = 1.0;

// Simulation properties
const size_t MAX_NUMBER_OF_MOLECULES = 1000;

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
	}

	inline Vector& operator-=(const Vector& vec)
	{
		x -= vec.x;
		y -= vec.y;
		z -= vec.z;
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
};

// Barnes-Hut Oct-Tree
// struct OctTreeNode
// {
// 	int oct000[2 * MAX_NUMBER_OF_MOLECULES];
// 	int oct001[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct010[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct011[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct100[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct101[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct110[2 * MAX_NUMBER_OF_MOLECULES]; 
// 	int oct111[2 * MAX_NUMBER_OF_MOLECULES];
// 	int moleculeIndex;
// 	PhysVal_t 
// }

// GAS MODEL CLASS
class GasModel
{
public:
	GasModel(PhysVal_t newMaxX, PhysVal_t newMaxY, PhysVal_t newMaxZ);

	void addMolecule(Vector coord, Vector speed, PhysVal_t radius, PhysVal_t mass);

	void move();
	void collideWithWalls();
	void collideWithEachOther();

	// Variables:
	PhysVal_t maxX, maxY, maxZ;

	size_t size;

	Vector      coords[MAX_NUMBER_OF_MOLECULES];
	Vector      speeds[MAX_NUMBER_OF_MOLECULES];
	PhysVal_t radiuses[MAX_NUMBER_OF_MOLECULES];
	PhysVal_t   masses[MAX_NUMBER_OF_MOLECULES];

	// Oct-tree
	// I'll code it up after the ideal gas model starts to work properly
};

GasModel::GasModel(PhysVal_t newMaxX, PhysVal_t newMaxY, PhysVal_t newMaxZ) :
	maxX (newMaxX),
	maxY (newMaxY),
	maxZ (newMaxZ),
	size (0),
	coords (), speeds (), radiuses (), masses ()
{}

void GasModel::addMolecule(Vector coord, Vector speed, PhysVal_t radius, PhysVal_t mass)
{
	if (size == MAX_NUMBER_OF_MOLECULES) return;

	  coords[size] = coord;
	  speeds[size] = speed;
	radiuses[size] = radius;
	  masses[size] = mass;

	  ++size;
}

void GasModel::move()
{
	for (size_t i = 0; i < size; ++i)
	{
		coords[i] += speeds[i];
	}
}

void GasModel::collideWithWalls()
{
	for (size_t i = 0; i < size; ++i)
	{
		Vector cur = coords[i];
		PhysVal_t rad = radiuses[i];

		cur.x = (cur.x < rad       )? 2 * rad          - cur.x :
		        (cur.x > maxX - rad)? 2 * (maxX - rad) - cur.x : cur.x;

		cur.y = (cur.y < rad       )? 2 * rad          - cur.y :
		        (cur.y > maxY - rad)? 2 * (maxY - rad) - cur.y : cur.y;

		cur.x = (cur.z < rad       )? 2 * rad          - cur.z :
		        (cur.z > maxZ - rad)? 2 * (maxZ - rad) - cur.z : cur.z;
	}
}

void GasModel::collideWithEachOther()
{
	for (size_t i = 0; i < size; ++i)
	{
		for (size_t j = i + 1; j < size; ++j)
		{
			PhysVal_t radSum = radiuses[i] + radiuses[j];
			Vector coordDiff = coords[i] - coords[j];

			if (coordDiff.lenSqr() > radSum * radSum) continue;

			// Shift out of collision:
			coordDiff.setLength(radSum);
			coords[i] = coords[j] + coordDiff;

			// Calculate new speeds:
			Vector centerOfMassSpeedx2 = (speeds[i] * masses[i] + speeds[j] * masses[j]) * (2/(masses[i] + masses[j]));

			speeds[i] = centerOfMassSpeedx2 - speeds[i];
			speeds[j] = centerOfMassSpeedx2 - speeds[j];
		}
	}
}

#endif  // GAS_MODEL_HPP_INCLUDED

