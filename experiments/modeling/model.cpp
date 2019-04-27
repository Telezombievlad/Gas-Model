#include "Model.h"
#include "SavingToFile.h"

#include <random>
#include <chrono>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("MODEL: Not enough arguments\n");
		return 1;
	}

	// Model
	GasModel model{{1000, 1000, 1000}};

	// A bit of code that saves model to file
	DataSaver saver{MAX_NUMBER_OF_MOLECULES};

	// Generating speeds from a distribution:
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::normal_distribution<PhysVal_t> speeds1{0,10};
	std::uniform_real_distribution<PhysVal_t> coords1{5,  10};
	std::uniform_real_distribution<PhysVal_t> coords2{5, 995};

	for (size_t i = 0; i < MAX_NUMBER_OF_MOLECULES; ++i)
	{
		Vector speed = {speeds1(gen), speeds1(gen), speeds1(gen)};
		Vector coord = {coords1(gen), coords2(gen), coords2(gen)};

		model.addMolecule(coord, speed);
	}

	std::chrono::steady_clock clock{};
	auto begin = clock.now();

	for (size_t i = 0; i < 900; ++i)
	{
		model.boxSize.x = 1000 + 800*sin(0.02*i);

		if (i % 1 == 0)
		{
			model.move();
			model.collideWithWalls();
			model.collideWithEachOther();
		}

		saver.writeFrame(model, argv[1], argv[2], false);
	}

	auto end = clock.now();

	std::chrono::nanoseconds diff = end - begin;
	printf("ELAPSED TIME = %f ms\n", diff.count() * 0.000001);
}
