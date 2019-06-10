// No Copyright. Maxim Manainen 2019
#include "Model.hpp"
#include "Dimensioning.hpp"
#include "cnpy.h"

#include <random>
#include <vector>

int prog_bar(int done, int whole) {
	std::cout << "\rPercent: " << done << "/" << whole;
	return 0;
}

int main(int argc, char * argv[]) {
	if (argc != 4) {
		std::cout << "Not enough arguments!" << "\n";
		return 1;
	}

	// Model
	GasModel model = GasModel({SAS_2_Model(1e3, 0, 1, 0),
	                           SAS_2_Model(1e3, 0, 1, 0),
	                           SAS_2_Model(1e3, 0, 1, 0)});

	// Generating speeds from a distribution:
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::normal_distribution<PhysVal_t>       speeds{SAS_2_Model(  0, -1, 1, 0), SAS_2_Model(2.5e13, -1, 1, 0)};
	std::uniform_real_distribution<PhysVal_t> coords{SAS_2_Model(250,  0, 1, 0), SAS_2_Model(   750,  0, 1, 0)};

	size_t num_molecules = std::stoi(argv[1]);

	for (size_t i = 0; i < num_molecules; ++i)
	{
		Vector speed = {speeds(gen), speeds(gen), speeds(gen)};
		Vector coord = {coords(gen), coords(gen), coords(gen)};

		model.addMolecule(Molecule(coord, speed, MoleculeType::HELIUM));
	}

	std::vector<PhysVal_t> res;

	int num_steps    = std::stoi(argv[2]);
	int display_step = std::stoi(argv[3]);

	for (int i = 0; i < num_steps; ++i)
	{
		model.iterationCycle();

		if (i % display_step == 0) {

			PhysVal_t speed = 0;

			for (size_t j = 0; j < num_molecules; ++j)
				speed += model.molecules[j].speed.lenSqr();
			res.push_back( speed / 2 );

			prog_bar(i, num_steps);
		}
	}

	long unsigned int size = num_steps / display_step;

	cnpy::npy_save("experiments/energy/kinetic.npy", &res[0], {1, size}, "w");

}
