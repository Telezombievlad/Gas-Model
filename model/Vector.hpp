// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_VECTOR_HPP_INCLUDED
#define GAS_MODEL_VECTOR_HPP_INCLUDED

#include <immintrin.h>

using PhysVal_t = double;

// 3D-vector with basic arithmetic
union Vector
{
	struct
	{
		PhysVal_t x, y, z;
	};
	__m256d reg256;
	int64_t conds[4];

	Vector() = default;

	Vector(PhysVal_t newX, PhysVal_t newY, PhysVal_t newZ);

	inline Vector& operator+=(const Vector& vec);
	inline Vector& operator-=(const Vector& vec);

	inline Vector operator+(const Vector& vec) const;
	inline Vector operator-(const Vector& vec) const;
	
	inline Vector& operator*=(PhysVal_t k);
	inline Vector& operator/=(PhysVal_t k);

	inline Vector operator*(PhysVal_t k) const;
	inline Vector operator/(PhysVal_t k) const;

	PhysVal_t lenSqr() const;
	PhysVal_t length() const;
	
	inline void setLength(PhysVal_t newLen);

	inline PhysVal_t scalar(const Vector& v) const;

	// It is written solely for <collide/attract>OneMoleculeBarnesHut - the bottleneck
	inline bool isInBox(Vector boxSize) const;
};

#endif  // GAS_MODEL_VECTOR_HPP_INCLUDED