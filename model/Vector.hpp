// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_VECTOR_HPP_INCLUDED
#define GAS_MODEL_VECTOR_HPP_INCLUDED

#include <cmath>
#include <immintrin.h>

using PhysVal_t = double;

const __m256d FLOATING_POINT_SIGN = _mm256_set1_pd(-0.0);

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

	Vector(PhysVal_t newX, PhysVal_t newY, PhysVal_t newZ) : 
		x (newX), y (newY), z (newZ)
	{}

	inline Vector& operator+=(const Vector& vec)
	{
		reg256 = _mm256_add_pd(reg256, vec.reg256);

		return *this;
	}

	inline Vector operator+(const Vector& vec) const
	{
		Vector toReturn;
		toReturn.reg256 = _mm256_add_pd(reg256, vec.reg256);
		
		return toReturn;
	}

	inline Vector& operator-=(const Vector& vec)
	{
		reg256 = _mm256_sub_pd(reg256, vec.reg256);

		return *this;
	}

	inline Vector operator-(const Vector& vec) const 
	{
		Vector toReturn;
		toReturn.reg256 = _mm256_sub_pd(reg256, vec.reg256);

		return toReturn;
	}
	
	inline Vector& operator*=(PhysVal_t k)
	{
		reg256 = _mm256_mul_pd(reg256, _mm256_set1_pd(k));

		return *this;
	}

	inline Vector operator*(PhysVal_t k) const
	{
		Vector toReturn;
		toReturn.reg256 = _mm256_mul_pd(reg256, _mm256_set1_pd(k));

		return toReturn;
	}

	inline Vector& operator/=(PhysVal_t k)
	{
		reg256 = _mm256_mul_pd(reg256, _mm256_set1_pd(1/k));

		return *this;
	}

	inline Vector operator/(PhysVal_t k) const
	{
		Vector toReturn;
		toReturn.reg256 = _mm256_mul_pd(reg256, _mm256_set1_pd(1/k));

		return toReturn;
	}

	inline PhysVal_t lenSqr() const
	{
		Vector len;
		len.reg256 = _mm256_mul_pd(reg256, reg256);

		return len.x + len.y + len.z;
	}

	inline PhysVal_t length() const
	{
		return std::sqrt(lenSqr());
	}

	inline void setLength(PhysVal_t newLen)
	{
		PhysVal_t curLen = length();

		if (std::abs(curLen) < 10 * std::numeric_limits<PhysVal_t>::epsilon())
		{
			x = curLen;
			y = 0;
			z = 0;

			return;
		}

		operator*=(newLen / curLen);
	}

	inline PhysVal_t scalar(const Vector& v) const
	{
		return v.x*x + v.y*y + v.z*z;
	}

	// It is written solely for <collide/attract>OneMoleculeBarnesHut - the bottleneck
	inline bool isInBox(Vector boxSize) const
	{
		Vector absoluteValue;
		absoluteValue.reg256 = _mm256_andnot_pd(FLOATING_POINT_SIGN, reg256);

		Vector conditions;
		conditions.reg256 = _mm256_cmp_pd(absoluteValue.reg256,  boxSize.reg256, _CMP_LT_OS);

		return conditions.conds[0] && conditions.conds[1] && conditions.conds[2];
	}
};

#endif  // GAS_MODEL_VECTOR_HPP_INCLUDED