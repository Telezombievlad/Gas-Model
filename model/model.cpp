#include "Model.hpp"
#include "SavingToFile.hpp"

#include <random>

int main(int argc, char* argv[])
{
	if (argc != 3)
	{
		printf("MODEL: Not enough arguments\n");
		return 1;
	}

	// Model
	GasModel model{1000, 1000, 1000};

	// A bit of code that saves model to file
	DataSaver saver{MAX_NUMBER_OF_MOLECULES};

	// Generating speeds from a distribution: 
	std::random_device rd;
    std::mt19937 gen{rd()};

    std::normal_distribution<PhysVal_t> speeds{0,5};
    std::uniform_real_distribution<PhysVal_t> coords{5, 15};

	for (size_t i = 0; i < MAX_NUMBER_OF_MOLECULES; ++i)
	{
		Vector speed = {speeds(gen), speeds(gen), speeds(gen)};
		Vector coord = {coords(gen), coords(gen), coords(gen)};

		model.addMolecule(coord, speed, 10, 1);
	}

	for (size_t i = 0; i < 600; ++i)
	{
		model.move();
		model.collideWithWalls();
		model.collideWithEachOther();

		saver.writeFrame(model, argv[1], argv[2]);
	}
}