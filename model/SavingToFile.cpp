// No Copyright. Vladislav Aleinik 2019
#include "SavingToFile.hpp"

#include "vendor/cnpy/cnpy.h"

const unsigned PRE_BUFFER_FACTOR = 10;

DataSaver::DataSaver(size_t count) :
	coords        (new PhysVal_t[PRE_BUFFER_FACTOR * 3 * count]),
	velocities    (new PhysVal_t[PRE_BUFFER_FACTOR *     count]),
	moleculeCount (count),
	preBufferred  (0)
{}

DataSaver::~DataSaver()
{
	delete[] coords;
	delete[] velocities; 
}

void DataSaver::writeFrame(const GasModel& model, const char* coordsFile, const char* velocitiesFile)
{
	for (size_t i = 0; i < moleculeCount; ++i)
	{
		coords[3 * (preBufferred * moleculeCount + i) + 0] = model.molecules[i].coords.x;
		coords[3 * (preBufferred * moleculeCount + i) + 1] = model.molecules[i].coords.y;
		coords[3 * (preBufferred * moleculeCount + i) + 2] = model.molecules[i].coords.z;
		
		velocities[preBufferred * moleculeCount + i] = model.molecules[i].speed.length();
	}

	preBufferred++;

	if (preBufferred == PRE_BUFFER_FACTOR)
	{
		preBufferred = 0;

		cnpy::npy_save(    coordsFile,     coords, {PRE_BUFFER_FACTOR, moleculeCount, 3}, "a");
		cnpy::npy_save(velocitiesFile, velocities, {PRE_BUFFER_FACTOR, moleculeCount   }, "a");
	}
}

void DataSaver::writeMoleculeTypes(const GasModel& model, const char* typesFile)
{
	char* moleculeTypes = new char[moleculeCount];

	for (size_t i = 0; i < moleculeCount; ++i)
	{
		moleculeTypes[i] = model.molecules[i].type;
	}

	cnpy::npy_save(typesFile, moleculeTypes, {moleculeCount}, "w");  
}
