.DEFAULT_GOAL := buildlib

#==================================================================================================
# COMPILER FLAGS
#==================================================================================================

CCFLAGS += -std=c++17 -Werror -Wall -O3 -mavx -fno-stack-protector

#==================================================================================================
# INSTALLATION
#==================================================================================================

install :
	# Vis libraries
	sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev
	sudo apt-get install python3-pyqt5
	pip3 install --upgrade vispy imageio

	# cnpy
	git clone https://github.com/rogersce/cnpy.git model/vendor/cnpy
	mkdir -p model/vendor/cnpy/build
	cd model/vendor/cnpy/build && cmake ..
	cd model/vendor/cnpy/build && sudo make && sudo make install

	mkdir -p model/bin

#==================================================================================================
# LIBRARY COMPILATION
#==================================================================================================

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR     := $(dir $(MKFILE_PATH))

SRC     = model
SRC_ABS = ${CUR_DIR}model
HEADERS = ${SRC}/Dimensioning.hpp ${SRC}/Model.hpp ${SRC}/Molecule.hpp ${SRC}/MoleculeTypes.hpp ${SRC}/SavingToFile.hpp ${SRC}/Vector.hpp ${SRC}/Walls.hpp
SOURCES = ${SRC}/Dimensioning.cpp ${SRC}/Model.cpp ${SRC}/Molecule.cpp ${SRC}/MoleculeTypes.cpp ${SRC}/SavingToFile.cpp ${SRC}/Vector.cpp ${SRC}/Walls.cpp

${SRC}/bin/unity.o : ${HEADERS} ${SOURCES}
	g++ -fPIC -c ${CCFLAGS} ${SRC}/unity.cpp -o ${SRC}/bin/unity.o

${SRC}/bin/libmodel.so : ${SRC}/bin/unity.o
	g++ -fPIC -shared ${CCFLAGS} -o ${SRC}/bin/libmodel.so ${SRC}/bin/unity.o

buildlib: ${SRC}/bin/libmodel.so
	@ echo "Library compiled!"

LINK_TO_CNPY_FLAGS = -L/usr/local -lcnpy -lz
LINK_TO_MODEL = -L${SRC_ABS}/bin -Wl,-rpath=${SRC_ABS}/bin -lmodel

#==================================================================================================
# VISUALIZATION
#==================================================================================================

VISUALIZE_SCRIPT = visualization/mol_vis.py

#==================================================================================================
# EXPERIMENTS
#==================================================================================================

######### Static modeling #########

MODEL_EXE = experiments/modeling/model
MODEL_SRC = experiments/modeling/model.cpp

model_compile : ${MODEL_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${MODEL_SRC} -I${SRC} -o ${MODEL_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

MODEL_COORDS     = experiments/modeling/1_coords.npy
MODEL_VELOCITIES = experiments/modeling/1_velocities.npy
MODEL_TYPES      = experiments/modeling/1_types.npy

model : model_compile
	rm -f ${MODEL_COORDS} ${MODEL_VELOCITIES} ${MODEL_TYPES}
	${MODEL_EXE} ${MODEL_COORDS} ${MODEL_VELOCITIES} ${MODEL_TYPES}

model_visualize :
	python3 ${VISUALIZE_SCRIPT} --cubesize 200x200x200 --realtime 1 --showtemp 1 ${MODEL_COORDS} ${MODEL_VELOCITIES} ${MODEL_TYPES}

######### Iso Processes #########

PROCESS_EXE = experiments/isoproc/isoproc
PROCESS_SRC = experiments/isoproc/isoproc.cpp

iso_compile: ${PROCESS_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${PROCESS_SRC} -I${SRC} -I${SRC}/vendor/cnpy -o ${PROCESS_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}


######### Diffusion #########

DIFF_EXE = experiments/diffusion/diffusion
DIFF_SRC = experiments/diffusion/diffusion.cpp

DIFF_COORDS     = experiments/diffusion/coords.npy
DIFF_VELOCITIES = experiments/diffusion/velocities.npy
DIFF_TYPES      = experiments/diffusion/types.npy
DIFF_CONC_HE    = experiments/diffusion/concHe.csv
DIFF_CONC_AR    = experiments/diffusion/concAr.csv
DIFF_FLUXES_HE  = experiments/diffusion/fluxesHe.npy
DIFF_FLUXES_AR  = experiments/diffusion/fluxesAr.npy
DIFF_GRADS_HE   = experiments/diffusion/gradsHe.npy
DIFF_GRADS_AR   = experiments/diffusion/gradsAr.npy

diffusion_compile : ${DIFF_SRC} ${SRC}/bin/libmodel.so
	g++ -g ${CCFLAGS} ${DIFF_SRC} -I${SRC} -I${SRC}/vendor/cnpy -o ${DIFF_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

DIFF_ARGS = ${DIFF_COORDS} ${DIFF_VELOCITIES} ${DIFF_TYPES}    \
            ${DIFF_CONC_HE} ${DIFF_CONC_AR} ${DIFF_FLUXES_HE}  \
            ${DIFF_FLUXES_AR} ${DIFF_GRADS_HE} ${DIFF_GRADS_AR}

diffusion : diffusion_compile
	rm -f ${DIFF_ARGS}
	${DIFF_EXE} ${DIFF_ARGS}

diffusion_visualize :
	python3 ${VISUALIZE_SCRIPT} ${DIFF_COORDS} ${DIFF_VELOCITIES} ${DIFF_TYPES} --cubesize 5000x1000x1000 --realtime 1 --showtemp 0 --koeff 7

######### Energy conservation #########

ENERGY_EXE = experiments/energy/energy
ENERGY_SRC = experiments/energy/energy.cpp

energy_compile : ${ENERGY_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${ENERGY_SRC} -I${SRC} -I${SRC}/vendor/cnpy -o ${ENERGY_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

ENERGY_MOLECULES    = 10000
ENERGY_STEPS        = 1000
ENERGY_DISLAY_EVERY = 1
energy : energy_compile
	${ENERGY_EXE} ${ENERGY_MOLECULES} ${ENERGY_STEPS} ${ENERGY_DISLAY_EVERY}

energy_visualize :
	echo "Energy visualization not done yet"

#==================================================================================================
# DEMOS
#==================================================================================================

######### Lennard-Jones Diffusion #########

DIFF_POTENTIAL_COORDS     = experiments/diffusion/POTENTIAL_coords.npy
DIFF_POTENTIAL_VELOCITIES = experiments/diffusion/POTENTIAL_velocities.npy
DIFF_POTENTIAL_TYPES      = experiments/diffusion/POTENTIAL_types.npy

demo_diffusion_potential :
	python3 ${VISUALIZE_SCRIPT} ${DIFF_POTENTIAL_COORDS} ${DIFF_POTENTIAL_VELOCITIES} ${DIFF_POTENTIAL_TYPES} --cubesize 5000x1000x1000 --realtime 1 --showtemp 0 --koeff 7

video_diffusion_potential :
	python3 ${VISUALIZE_SCRIPT} ${DIFF_POTENTIAL_COORDS} ${DIFF_POTENTIAL_VELOCITIES} ${DIFF_POTENTIAL_TYPES} --cubesize 5000x1000x1000 --realtime 0 --showtemp 0 --koeff 7

######### Bouncy-Ball Diffusion #########

DIFF_BOUNCY_COORDS     = experiments/diffusion/BOUNCY_coords.npy
DIFF_BOUNCY_VELOCITIES = experiments/diffusion/BOUNCY_velocities.npy
DIFF_BOUNCY_TYPES      = experiments/diffusion/BOUNCY_types.npy

demo_diffusion_bouncy :
	python3 ${VISUALIZE_SCRIPT} ${DIFF_BOUNCY_COORDS} ${DIFF_BOUNCY_VELOCITIES} ${DIFF_BOUNCY_TYPES} --cubesize 5000x1000x1000 --realtime 0 --showtemp 0 --koeff 7

######### Floating Liquid #########

LIQUID_COORDS     = experiments/modeling/LIQUID_coords.npy
LIQUID_VELOCITIES = experiments/modeling/LIQUID_velocities.npy
LIQUID_TYPES      = experiments/modeling/LIQUID_types.npy

demo_liquid :
	python3 ${VISUALIZE_SCRIPT} --cubesize 100x100x100 --realtime 1 --showtemp 0 ${LIQUID_COORDS} ${LIQUID_VELOCITIES} ${LIQUID_TYPES}

video_liquid :
	python3 ${VISUALIZE_SCRIPT} --cubesize 100x100x100 --realtime 0 --showtemp 0 ${LIQUID_COORDS} ${LIQUID_VELOCITIES} ${LIQUID_TYPES}


######### Liquid In Gravitational  #########

GRAVITY_COORDS     = experiments/modeling/GRAVITY_coords.npy
GRAVITY_VELOCITIES = experiments/modeling/GRAVITY_velocities.npy
GRAVITY_TYPES      = experiments/modeling/GRAVITY_types.npy

demo_gravity :
	python3 ${VISUALIZE_SCRIPT} --cubesize 200x200x200 --realtime 1 --showtemp 1 ${GRAVITY_COORDS} ${GRAVITY_VELOCITIES} ${GRAVITY_TYPES}

video_gravity :
	python3 ${VISUALIZE_SCRIPT} --cubesize 200x200x200 --realtime 0 --showtemp 1 ${GRAVITY_COORDS} ${GRAVITY_VELOCITIES} ${GRAVITY_TYPES}

######### 50000 Bouncy-Ball Wave  #########

WAVES_COORDS     = experiments/modeling/WAVES_coords.npy
WAVES_VELOCITIES = experiments/modeling/WAVES_velocities.npy
WAVES_TYPES      = experiments/modeling/WAVES_types.npy

demo_waves :
	python3 ${VISUALIZE_SCRIPT} --fps 15 --cubesize 150x150x150 --realtime 1 --showtemp 1 ${WAVES_COORDS} ${WAVES_VELOCITIES} ${WAVES_TYPES}

video_waves :
	python3 ${VISUALIZE_SCRIPT} --fps 15 --cubesize 150x150x150 --realtime 0 --showtemp 1 ${WAVES_COORDS} ${WAVES_VELOCITIES} ${WAVES_TYPES}

#==================================================================================================
# OPTIMIZATION
#==================================================================================================

MODEL_ASM = optimization/listing.asm
OPTIMIZE_HDRS = model/Model.hpp model/Model_SSE.hpp model/Model_BARNES_HUT.hpp model/Model_HARDCORE.hpp

compile_debug : ${MODEL_SRC} ${MODEL_HDRS}
	g++ -S ${CCFLAGS} -g ${MODEL_SRC} -o ${MODEL_ASM} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}
	g++    ${CCFLAGS} -g ${MODEL_SRC} -o ${MODEL_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

profile : compile_debug
	rm -f ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}

#==================================================================================================
# MISCELLANEOUS
#==================================================================================================

clean:
	rm -f ${SRC}/bin/unity.o ${SRC}/bin/libmodel.so experiments/modeling/model