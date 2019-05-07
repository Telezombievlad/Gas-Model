#include "Model.h"
#include "Dimensioning.h"
#include "cnpy.h"

#include <random>
#include <vector>

int prog_bar(int done, int whole) {
	std::cout << "\rPercent: " << done << "/" << whole;
	return 0;
}
int main(int argc, char * argv[])
{

	if (argc != 4) {
		std::cout << "Not enough arguments!" << "\n";
		return 1;
	}

	GasModel model{{1000, 1000, 1000}};

	// Generating speeds from a distribution:
	std::random_device rd;
  std::mt19937 gen{rd()};

  std::normal_distribution<PhysVal_t> speeds{0,1};
  std::uniform_real_distribution<PhysVal_t> coords{250, 750};

	size_t num_molecules = std::stoi(argv[1]);

	for (size_t i = 0; i < num_molecules; ++i)
	{
		Vector speed = {speeds(gen), speeds(gen), speeds(gen)};
		Vector coord = {coords(gen), coords(gen), coords(gen)};

		model.addMolecule(coord, speed);
	}

	std::vector<PhysVal_t> res;

	int num_steps = std::stoi(argv[2]);
	int display_step = std::stoi(argv[3]);

	for (int i = 0; i < num_steps; ++i)
	{
		model.move();
		model.collideWithWalls();
		model.collideWithEachOther();

		if (i % display_step == 0) {

			PhysVal_t speed = 0;

			for (size_t j = 0; j < num_molecules; ++j)
				speed += model.speeds[j].length() * model.speeds[j].length();
			res.push_back( speed / 2 );

			prog_bar(i, num_steps);
		}
	}

	long unsigned int size = num_steps / display_step;

	cnpy::npy_save("experiments/energy/kinetic.npy", &res[0], {1, size}, "a");

}
