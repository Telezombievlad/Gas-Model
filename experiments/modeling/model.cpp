// No Copyright. Vladislav Aleinik 2019
#include "Model.hpp"
#include "SavingToFile.hpp"

#include <random>
#include <chrono>

#include <fenv.h>

const size_t MOLECULES        = 20000;
const size_t ITERATIONS       = 10000;
const size_t SAVE_FRAME_EVERY = 5; 
const size_t FIX_T_EVERY      = 100;

int main(int argc, char* argv[])  
{
	if (argc != 4)
	{
		printf("MODEL: Not enough arguments\n");
		printf("Call pattern: model <.npy coordinates> <.npy velocities> <.npy molecule types>\n");
		return 1;
	}

	// Model
	const PhysVal_t BOX_SIZE = SAS_2_Model(2e2, 0, 1, 0);
	GasModel model = GasModel({BOX_SIZE, BOX_SIZE, BOX_SIZE});

	// A bit of code that saves model to file
	DataSaver saver{MOLECULES};

	// Generating speeds from a distribution:
	std::random_device rd;
	std::mt19937 gen{rd()};

	static const PhysVal_t BOLTZMANN_K       = 1.38e-23;
	static const PhysVal_t ATOMIC_MASS_IN_KG = 1.67e-27;
	const PhysVal_t TEMPERATURE              = 300/*K*/;
	const PhysVal_t sigmaHe = SAS_2_Model(std::sqrt(BOLTZMANN_K*TEMPERATURE/(ATOMIC_MASS_IN_KG*MASSES[ARGON])) * 1e10, -1, 1, 0);

	std::normal_distribution<PhysVal_t>       speeds  {0.0, sigmaHe};
	std::uniform_real_distribution<PhysVal_t> coordsZ {0.0, BOX_SIZE};
	std::uniform_real_distribution<PhysVal_t> coordsXY{0.0, BOX_SIZE};

	// Filling array of molecules
	for (size_t i = 0; i < MOLECULES; ++i)
	{
		Vector speed = Vector(speeds  (gen), speeds  (gen), speeds (gen));
		Vector coord = Vector(coordsXY(gen), coordsXY(gen), coordsZ(gen));

		model.addMolecule(Molecule(coord, speed, MoleculeType::HELIUM));
	}

	// Write molecule types to file
	saver.writeMoleculeTypes(model, argv[3]);

	// Init timers
	std::chrono::steady_clock clock{};
	auto begin = clock.now();

	// THE SIMULATION
	for (size_t iter = 0; iter < ITERATIONS; ++iter)  
	{
		if (iter % FIX_T_EVERY == 0)
			model.fixEnergy();

		if (iter % SAVE_FRAME_EVERY == 0)
		{
			printf("\r[MODEL] %03zu%%", 100*iter/ITERATIONS);
			std::fflush(stdout);

			saver.writeFrame(model, argv[1], argv[2]);
		}

		model.iterationCycle();
	}

	auto end = clock.now();

	std::chrono::nanoseconds diff = end - begin;

	printf("SIMULATION TIME = %9.3f ms\n", diff.count() * 0.000001);

	return EXIT_SUCCESS;
}

