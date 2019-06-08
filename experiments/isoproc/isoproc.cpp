#include <iostream>
#include <random>

#include "Model.hpp"
#include "Dimensioning.hpp"

int main() {

  float cube_s = 1000;
  size_t NUMBER_OF_MOLECULES = 10000;

  //int tree_depth = 1;

  GasModel model{{cube_s, cube_s, cube_s}};

  // Generating speeds from a distribution:
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::normal_distribution<PhysVal_t> speeds1{0,10};
	std::uniform_real_distribution<PhysVal_t> coords1{5,  10};
	std::uniform_real_distribution<PhysVal_t> coords2{5, 995};

	for (size_t i = 0; i < NUMBER_OF_MOLECULES; ++i)
	{
		Vector speed = {speeds1(gen), speeds1(gen), speeds1(gen)};
		Vector coord = {coords1(gen), coords2(gen), coords2(gen)};

		model.addMolecule(coord, speed);
	}

	model.buildOctTree();

	std::cout << model.octTree[0].center.x << "\n";

	float impulse[static_cast<int>( std::pow(8 * tree_depth) )];

	for (size_t i = 0; i < 900; ++i)
	{
		model.boxSize.x = 1000 + 800*sin(0.02*i);

		if (i % 1 == 0)
		{
			model.iterationCycle();
		}
	}

}
