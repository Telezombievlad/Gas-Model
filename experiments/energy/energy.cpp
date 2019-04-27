#include "../../model/Model.hpp"

#include <random>

int main()
{
	GasModel model{1000, 1000, 1000};

	// Generating speeds from a distribution:
	std::random_device rd{};
    std::mt19937 gen{rd()};

    std::normal_distribution<PhysVal_t> speeds{0,1};
    std::uniform_real_distribution<PhysVal_t> coords{250, 750};

	for (size_t i = 0; i < 10; ++i)
	{
		Vector speed = {speeds(gen), speeds(gen), speeds(gen)};
		Vector coord = {coords(gen), coords(gen), coords(gen)};

		model.addMolecule(coord, speed, 10, 1);
	}

	while (true)
	{
		model.move();
		model.collideWithWalls();
		model.collideWithEachOther();
	}
}
