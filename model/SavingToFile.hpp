// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED
#define GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED

#include "vendor/cnpy/cnpy.h"
#include "Model.hpp"

#include <vector>

class DataSaver
{
private:
	PhysVal_t* coords;
	PhysVal_t* velocities;
	size_t molecules;

public:
	DataSaver(size_t moleculeCount) :
		coords     (new PhysVal_t[3 * moleculeCount]),
		velocities (new PhysVal_t[moleculeCount]),
		molecules  (moleculeCount)
	{}

	~DataSaver()
	{
		delete[] coords;
		delete[] velocities;
	}

	void writeFrame(const GasModel& model, const char* coordsFile, const char* velocitiesFile, bool octTree)
	{		

		if (!octTree)
		{
			for (size_t i = 0; i < molecules; ++i)
			{
				coords[3 * i + 0] = model.coords[i].x - 0.5*model.boxSize.x;
				coords[3 * i + 1] = model.coords[i].y - 0.5*model.boxSize.y;  
				coords[3 * i + 2] = model.coords[i].z - 0.5*model.boxSize.z;
				velocities[i] = model.speeds[i].length();
			}
		}
		else
		{
			for (size_t i = 0; i < molecules; ++i)
			{
				if (i < model.octTreeSize)
				{
					coords[3 * i + 0] = model.octTree[i].center.x - 0.5*model.boxSize.x;
					coords[3 * i + 1] = model.octTree[i].center.y - 0.5*model.boxSize.y;  
					coords[3 * i + 2] = model.octTree[i].center.z - 0.5*model.boxSize.z;
					velocities[i] = model.octTree[i].count;
				}
				else
				{
					coords[3 * i + 0] = 0;
					coords[3 * i + 1] = 0;
					coords[3 * i + 2] = 0;					
					velocities[i] = 0;
				}
			}
		}

		cnpy::npy_save(    coordsFile,     coords, {1, molecules, 3}, "a");
		cnpy::npy_save(velocitiesFile, velocities, {1, molecules   }, "a");
	}
};

#endif // GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED