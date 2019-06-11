// No Copyright. Vladislav Aleinik 2019
#include "Model.hpp"
#include "SavingToFile.hpp"
#include "cnpy.h"

#include <valarray>
#include <numeric>
#include <random>

const size_t MOLECULES        = MAX_NUMBER_OF_MOLECULES;
const size_t ITERATIONS       = 3000;
const size_t SAVE_FRAME_EVERY = 10;
const size_t SAVE_EVERY       = 100;
const size_t BIN_COUNT        = 10;

int main(int argc, char* argv[])
{
	if (argc != 10)
	{
		printf("DIFFUSION: Wrong arguments\n");
		printf("Call pattern: diffusion <.npy coords> <.npy speeds> <.npy mol types> "
		       "<.csv concHe> <.csv concAr> <.npy fluxHe> <.npy fluxAr> "
		       "<.npy gradsHe> <.npy gradsAr>\n");
		return 1;
	}

	// Model
	const PhysVal_t ACTUAL_BOX_SIZE = 1e3;
	const PhysVal_t BOX_SIZE = SAS_2_Model(ACTUAL_BOX_SIZE, 0, 1, 0);
	GasModel model = GasModel({BOX_SIZE, BOX_SIZE, BOX_SIZE});

	// Generating speeds and coordinates:
	std::random_device rd;
	std::mt19937 gen{rd()};

	const PhysVal_t temp = 300/*K*/;
	const PhysVal_t sigmaHe = SAS_2_Model(std::sqrt(1.38e-23*temp/(1.67e-27*MASSES[HELIUM])) * 1e-10, -1, 1, 0);
	const PhysVal_t sigmaAr = SAS_2_Model(std::sqrt(1.38e-23*temp/(1.67e-27*MASSES[ ARGON])) * 1e-10, -1, 1, 0);

	std::normal_distribution<PhysVal_t> speedPrjHe{0, sigmaHe};
	std::normal_distribution<PhysVal_t> speedPrjAr{0, sigmaAr};

	std::uniform_real_distribution<PhysVal_t> coordsXHe{0.0*BOX_SIZE, 0.5*BOX_SIZE};
	std::uniform_real_distribution<PhysVal_t> coordsXAr{0.5*BOX_SIZE, 1.0*BOX_SIZE};
	std::uniform_real_distribution<PhysVal_t> coordsYZ {0, BOX_SIZE};

	// Filling array of molecules
	for (size_t i = 0; i < MOLECULES/2; ++i)
	{
		Vector speed = Vector(speedPrjHe(gen), speedPrjHe(gen), speedPrjHe(gen));
		Vector coord = Vector( coordsXHe(gen),   coordsYZ(gen),   coordsYZ(gen));

		model.addMolecule(Molecule(coord, speed, MoleculeType::HELIUM));
	}

	for (size_t i = MOLECULES/2; i < MOLECULES; ++i)
	{
		Vector speed = Vector(speedPrjAr(gen), speedPrjAr(gen), speedPrjAr(gen));
		Vector coord = Vector( coordsXAr(gen),   coordsYZ(gen),   coordsYZ(gen));

		model.addMolecule(Molecule(coord, speed, MoleculeType::ARGON));
	}

	// Saving data
	DataSaver saver{MOLECULES};
	saver.writeMoleculeTypes(model, argv[3]);

	FILE* concentrationsHeHandle = fopen(argv[4], "w");
	if (concentrationsHeHandle == nullptr)
	{
		printf("DIFFUSION: Unable to open file \'%s\'\n", argv[4]);
	}

	FILE* concentrationsArHandle = fopen(argv[5], "w");
	if (concentrationsArHandle == nullptr)
	{
		printf("DIFFUSION: Unable to open file \'%s\'\n", argv[4]);
	}

	// Stuff to analyse diffusion:
	auto countsHeCur = std::valarray<unsigned>(BIN_COUNT);
	auto countsHePrv = std::valarray<unsigned>(BIN_COUNT);
	auto countsArCur = std::valarray<unsigned>(BIN_COUNT);
	auto countsArPrv = std::valarray<unsigned>(BIN_COUNT);

	auto fluxesHe = std::valarray<PhysVal_t>(BIN_COUNT - 1);
	auto fluxesAr = std::valarray<PhysVal_t>(BIN_COUNT - 1);

	auto gradsHe = std::valarray<PhysVal_t>(BIN_COUNT - 1);
	auto gradsAr = std::valarray<PhysVal_t>(BIN_COUNT - 1);

	// THE SIMULATION
	for (size_t iter = 0; iter <= ITERATIONS; ++iter)
	{
		if (iter % SAVE_FRAME_EVERY == 0)
		{
			printf("\r[DIFFUSION] %03zu%%", 100*iter/ITERATIONS);
			std::fflush(stdout);
		}

		model.iterationCycle();

		if (iter % SAVE_FRAME_EVERY == 0)
			saver.writeFrame(model, argv[1], argv[2]);

		if (iter % SAVE_EVERY == 0)
		{
			// Filling arrays
			countsHePrv = countsHeCur;
			countsArPrv = countsArCur;

			countsHeCur = 0;
			countsArCur = 0;

			for (size_t mol = 0; mol < MOLECULES; ++mol)
			{
				size_t index = lround(floor(10 * model.molecules[mol].coords.x / BOX_SIZE)) % 10;

				if (model.molecules[mol].type == HELIUM) countsHeCur[index] += 1;
				else                                     countsArCur[index] += 1;
			}

			fluxesHe = 0.0;
			fluxesAr = 0.0;

			for (size_t fluxI = 0; fluxI < BIN_COUNT-1; ++fluxI)
			{
				for (size_t binJ = fluxI; binJ < BIN_COUNT-1; ++binJ)
				{
					fluxesHe[fluxI] += countsHeCur[binJ] - countsHePrv[binJ];
					fluxesAr[fluxI] += countsArCur[binJ] - countsArPrv[binJ];
				}

				gradsHe[fluxI] = countsHeCur[fluxI+1] - countsHeCur[fluxI];
				gradsAr[fluxI] = countsArCur[fluxI+1] - countsArCur[fluxI];
			}

			// Bringing data back to normal dimensions
			fluxesHe /= std::pow(ACTUAL_BOX_SIZE, 2) * (TIME_DELTA * SAVE_EVERY);
			fluxesAr /= std::pow(ACTUAL_BOX_SIZE, 2) * (TIME_DELTA * SAVE_EVERY);

			gradsHe /= std::pow(ACTUAL_BOX_SIZE, 3) * (ACTUAL_BOX_SIZE / BIN_COUNT);
			gradsAr /= std::pow(ACTUAL_BOX_SIZE, 3) * (ACTUAL_BOX_SIZE / BIN_COUNT);

			// Saving stuff to file
			for (size_t i = 0; i < BIN_COUNT; ++i)
			{
				if (i != 0)
				{
					fprintf(concentrationsHeHandle, ", ");
					fprintf(concentrationsArHandle, ", ");
				}

				fprintf(concentrationsHeHandle, "%f", countsHeCur[i] / std::pow(ACTUAL_BOX_SIZE, 3));
				fprintf(concentrationsArHandle, "%f", countsArCur[i] / std::pow(ACTUAL_BOX_SIZE, 3));
			}

			fprintf(concentrationsHeHandle, "\n");
			fprintf(concentrationsArHandle, "\n");

			if (iter != 0)
			{
				cnpy::npy_save(argv[6], &fluxesHe[0], {BIN_COUNT-1}, "a");
				cnpy::npy_save(argv[7], &fluxesAr[0], {BIN_COUNT-1}, "a");
				cnpy::npy_save(argv[8],  &gradsHe[0], {BIN_COUNT-1}, "a");
				cnpy::npy_save(argv[9],  &gradsAr[0], {BIN_COUNT-1}, "a");
			}
		}
	}

	fclose(concentrationsHeHandle);
	fclose(concentrationsArHandle);

	printf("\n");

	return EXIT_SUCCESS;
}
