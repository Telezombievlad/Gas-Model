#include "Model.hpp"
#include "SavingToFile.hpp"

#include <random>
#include <chrono>

#include <fenv.h>

int main(int argc, char* argv[])
{
	if (argc != 4)
	{
		printf("MODEL: Not enough arguments\n"); 
		return 1;
	}

	// Model
	GasModel model = GasModel({SI_2_Model(1000.0,  0, 1, 0),
	                           SI_2_Model(1000.0,  0, 1, 0),
	                           SI_2_Model(1000.0,  0, 1, 0)});

	// This object saves data to file
	DataSaver saver{MAX_NUMBER_OF_MOLECULES}; 

	// Generating speeds from a distribution: 
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::normal_distribution<PhysVal_t>       speeds1{SI_2_Model( 0.0, -1, 1, 0), SI_2_Model(  5.0, -1, 1, 0)};
	std::uniform_real_distribution<PhysVal_t> coords1{SI_2_Model( 5.0,  0, 1, 0), SI_2_Model( 10.0,  0, 1, 0)};
	std::uniform_real_distribution<PhysVal_t> coords2{SI_2_Model(50.0,  0, 1, 0), SI_2_Model(950.0,  0, 1, 0)};

	// Filling array of molecules
	for (size_t i = 0; i < MAX_NUMBER_OF_MOLECULES; ++i)
	{
		Vector speed = Vector(speeds1(gen), speeds1(gen), speeds1(gen));
		Vector coord = Vector(coords1(gen), coords2(gen), coords2(gen));

		model.addMolecule(Molecule(coord, speed, MoleculeType::HELIUM));
	}

	//model.addMolecule(Molecule(Vector(450, 500, 500), Vector( 1, 0, 0), MoleculeType::HELIUM));
	//model.addMolecule(Molecule(Vector(550, 500, 500), Vector(-1, 0, 0), MoleculeType::HELIUM));

	// Write molecule types to file
	saver.writeMoleculeTypes(model, argv[3]);

	// Init timers
	std::chrono::steady_clock clock{};
	auto begin = clock.now();

	// THE SIMULATION
	for (size_t i = 0; i < 900; ++i)
	{
		// model.boxSize.x = 1000 + 800*sin(0.02*i);
		
		model.iterationCycle();

		saver.writeFrame(model, argv[1], argv[2]);
	}

	auto end = clock.now();

	std::chrono::nanoseconds diff = end - begin;
	printf("SIMULATION TIME = %9.3f ms\n", diff.count() * 0.000001);
}