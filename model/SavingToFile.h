// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_SAVING_TO_FILE_INCLUDED
#define GAS_MODEL_SAVING_TO_FILE_INCLUDED

#include "vendor/cnpy/cnpy.h"
#include "Model.h"

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

	~DataSaver();

	void writeFrame(const GasModel& model, const char* coordsFile, const char* velocitiesFile, bool octTree);
  
};

#endif // GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED
