// No Copyright. Vladislav Aleinik 2019
#ifndef GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED
#define GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED

#include "Model.hpp"

class DataSaver
{
private:
	PhysVal_t* coords;
	PhysVal_t* velocities;
	size_t moleculeCount;
	size_t preBufferred;

public:
	DataSaver(size_t count);
	~DataSaver();

	void writeFrame(const GasModel& model, const char* coordsFile, const char* velocitiesFile);
	void writeMoleculeTypes(const GasModel& model, const char* typesFile);
};

#endif // GAS_MODEL_SAVING_TO_FILE_HPP_INCLUDED