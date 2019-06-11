#include <iostream>
#include <random>
#include <cmath>

#include "Model.h"
#include "Dimensioning.h"

std::pair<int, int> count_index(size_t molecule_id, const GasModel& model, int max_depth) {

  int tree_idx = 0, normalized_idx = 0;
  char oct = 0;

  for (int i = 0; i < max_depth; ++i) {

    oct = model.calculateOct(molecule_id, tree_idx);

    tree_idx = model.octTree[tree_idx].octs[0 + oct];
    normalized_idx = 8 * normalized_idx + (oct + 0);
  }

  return std::make_pair(tree_idx, normalized_idx);
}

int detect_collision(Vector mol, PhysVal_t radius, Vector center, Vector size) {

  Vector offset = (center - size * 0.5) - mol;
  Vector overset = (center + size * 0.5) - mol;

  if (abs(offset.x) < radius || abs(overset.x) < radius)
  {
    return 1;
  }
  else if (abs(offset.y) < radius || abs(overset.y) < radius)
  {
    return 2;
  }

  if (abs(offset.z) < radius || abs(overset.z) < radius)
  {
    return 3;
  }
  return 0;

}

PhysVal_t* get_pressure(const GasModel& model, int depth, size_t NUMBER_OF_MOLECULES) {

  PhysVal_t* impulse = new PhysVal_t[static_cast<int>( std::pow(8, depth) )]{0};

  int collis = 0;
  std::pair<int, int> idx;

  for (size_t i = 0; i < NUMBER_OF_MOLECULES; ++i) {

    idx = count_index(i, model, depth);
    collis = detect_collision(model.coords[i], RADIUS, model.octTree[idx.first].center, model.sizeAtDepth[depth]);

    switch(collis) {
      case 1:
        impulse[idx.second] += abs(model.speeds[i].x);
        break;
      case 2:
        impulse[idx.second] += abs(model.speeds[i].y);
        break;
      case 3:
        impulse[idx.second] += abs(model.speeds[i].z);
        break;
      default:
        break;
    }
  }

  for (int i = 0; i < static_cast<int>( std::pow(8, depth) ); ++i)
    impulse[i] = Model_2_SI(impulse[i] * 1 / 1 / std::pow(model.sizeAtDepth[depth].x, 2), -2, -1, 1);

  return impulse;
}

PhysVal_t* get_temp(const GasModel& model, int depth, size_t NUMBER_OF_MOLECULES) {

  PhysVal_t* speeds = new PhysVal_t[static_cast<int>( std::pow(8, depth) )]{0};
  size_t number[static_cast<int>( std::pow(8, depth) )] = {0};

  std::pair<int, int> idx;

  for (size_t i = 0; i < NUMBER_OF_MOLECULES; ++i) {

    idx = count_index(i, model, depth);

    speeds[idx.second] += model.speeds[i].lenSqr();
    number[idx.second] += 1;
  }

  for (int i = 0; i < static_cast<int>( std::pow(8, depth) ); ++i)
    speeds[i] = Model_2_SI( speeds[i]/number[i] * 1 / 2, -2, 2, 1) * 2/3 /1.4 * 1e23;

  return speeds;
}

int main() {

  float cube_s = 1;
  size_t NUMBER_OF_MOLECULES = 10000;

  int max_depth = 1;

  GasModel model{{SI_2_Model(cube_s, 0, 1, 0),
                  SI_2_Model(cube_s, 0, 1, 0),
                  SI_2_Model(cube_s, 0, 1, 0),}};

  // Model initialization
	std::random_device rd;
	std::mt19937 gen{rd()};

	std::normal_distribution<PhysVal_t> speeds1{0, SI_2_Model(2500, -1, 1, 0)};
	std::uniform_real_distribution<PhysVal_t> coords1{0,  model.boxSize.x/2};
	std::uniform_real_distribution<PhysVal_t> coords2{0, model.boxSize.y/2};

	for (size_t i = 0; i < NUMBER_OF_MOLECULES; ++i)
	{
		Vector speed = {speeds1(gen), speeds1(gen), speeds1(gen)};
		Vector coord = {coords1(gen), coords2(gen), coords2(gen)};

		model.addMolecule(coord, speed);
	}
  model.buildOctTree();

  // Main action baby

  std::cout << model.boxSize.x << " " << model.boxSize.y << " " << model.boxSize.z << "\n";

  std::cout << "Our volume: " << Model_2_SI(model.boxSize.x*model.boxSize.y*model.boxSize.z, 0, 3, 0) << "\n";
  std::cout << "Qunatity of stuff: " << NUMBER_OF_MOLECULES / 6;

  PhysVal_t* pres;
  PhysVal_t* temp;

  for (size_t i = 0; i < 900; ++i)
  {
    //model.boxSize.x = 1000 + 800*sin(0.02*i);

    if (i % 1 == 0)
    {
      model.move();
      model.collideWithWalls();
      model.collideWithEachOther();
    }

    if (i % 100 == 0) {

      std::cout << i << "\n";

      pres = get_pressure(model, max_depth, NUMBER_OF_MOLECULES);
      temp = get_temp(model, max_depth, NUMBER_OF_MOLECULES);

      for(int i = 0; i < static_cast<int>( std::pow(8, max_depth) ); ++i) {
        std::cout << pres[i] << " " << temp[i] << "\n";
      }

      delete[] pres;
      delete[] temp;
    }
  }

  return 0;
}
