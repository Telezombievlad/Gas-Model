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

	void writeFrame(const GasModel& model, const char* coordsFile, const char* velocitiesFile)
	{		
		for (size_t i = 0; i < molecules && i < model.size; ++i)
		{
			coords[3 * i + 0] = model.coords[i].x - 0.5*model.maxX;
			coords[3 * i + 1] = model.coords[i].y - 0.5*model.maxY;  
			coords[3 * i + 2] = model.coords[i].z - 0.5*model.maxZ;
			velocities[i] = model.speeds[i].length();
		}

		cnpy::npy_save(    coordsFile,     coords, {1, molecules, 3}, "a");
		cnpy::npy_save(velocitiesFile, velocities, {1, molecules   }, "a");
	}
};

#endif // GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED