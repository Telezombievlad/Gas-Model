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

RENDER_MODE = --cubesize 1000x1000x1000 --realtime 1 --showtemp 0

#==================================================================================================
# EXPERIMENTS
#==================================================================================================

######### Static modeling #########

MODEL_EXE = experiments/modeling/model
MODEL_SRC = experiments/modeling/model.cpp

MODEL_COORDS     = experiments/modeling/coords.npy
MODEL_VELOCITIES = experiments/modeling/velocities.npy
MODEL_TYPES      = experiments/modeling/types.npy

model_compile : ${MODEL_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${MODEL_SRC} -I${SRC} -o ${MODEL_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

model : model_compile
	rm -f ${MODEL_COORDS} ${MODEL_VELOCITIES} ${MODEL_TYPES}
	${MODEL_EXE} ${MODEL_COORDS} ${MODEL_VELOCITIES} ${MODEL_TYPES}

model_visualize :
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} ${MODEL_COORDS} ${MODEL_VELOCITIES}

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

demo_energy :
	echo "Energy visualization not done yet"

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
	python3 ${VISUALIZE_SCRIPT} ${DIFF_COORDS} ${DIFF_VELOCITIES} ${DIFF_TYPES} ${RENDER_MODE}

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
