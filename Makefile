.DEFAULT_GOAL := default

#==================================================================================================
# COMPILER FLAGS
#==================================================================================================

CCFLAGS += -std=c++17 -Werror -Wall -O3 -mavx -fno-stack-protector

#==================================================================================================
# LIBRARY INSTALLATION
#==================================================================================================

install :
	# Vis libraries
	sudo apt-get install libglu1-mesa-dev freeglut3-dev mesa-common-dev
	sudo apt-get install python3-pyqt5
	pip install --upgrade vispy imageio

	# cnpy
	git clone https://github.com/rogersce/cnpy.git model/vendor/cnpy
	mkdir model/vendor/cnpy/build
	cd model/vendor/cnpy/build && cmake ..
	cd model/vendor/cnpy/build && sudo make && sudo make install

	mkdir model/bin

MKFILE_PATH := $(abspath $(lastword $(MAKEFILE_LIST)))
CUR_DIR     := $(dir $(MKFILE_PATH))

SRC     = model
SRC_ABS = ${CUR_DIR}/model
HEADERS = ${SRC}/Dimensioning.hpp ${SRC}/Model.hpp ${SRC}/Molecule.hpp ${SRC}/MoleculeTypes.hpp ${SRC}/SavingToFile.hpp ${SRC}/Vector.hpp ${SRC}/Walls.hpp
SOURCES = ${SRC}/Dimensioning.cpp ${SRC}/Model.cpp ${SRC}/Molecule.cpp ${SRC}/MoleculeTypes.cpp ${SRC}/SavingToFile.cpp ${SRC}/Vector.cpp ${SRC}/Walls.cpp

${SRC}/bin/unity.o : ${HEADERS} ${SOURCES}
	g++ -fPIC -c ${CCFLAGS} ${SRC}/unity.cpp -o ${SRC}/bin/unity.o

${SRC}/bin/libmodel.so : ${SRC}/bin/unity.o
	g++ -fPIC -shared ${CCFLAGS} -o ${SRC}/bin/libmodel.so ${SRC}/bin/unity.o

default: ${SRC}/bin/libmodel.so         
	@ echo "Library compiled!"

#set -x LD_LIBRARY_PATH ~/Dropbox/Programming/2018-2019/gas/model/bin ${LD_LIBRARY_PATH}

#==================================================================================================
# EXPERIMENTS
#==================================================================================================

######### Static modeling #########

MODEL_EXE   = experiments/modeling/model
MODEL_SRC   = experiments/modeling/model.cpp

LINK_TO_CNPY_FLAGS = -L/usr/local -lcnpy -lz
LINK_TO_MODEL = -L${SRC_ABS}/bin -Wl,-rpath=${SRC_ABS}/bin -lmodel

compile_model : ${MODEL_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${MODEL_SRC} -I${SRC} -o ${MODEL_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

model : compile_model
	rm -f ${RES_COORDS} ${RES_VELOCITIES}
	${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}

######### Energy conservation #########

ENERGY_EXE = experiments/energy/energy
ENERGY_SRC = experiments/energy/energy.cpp

compile_energy : ${ENERGY_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${ENERGY_SRC} -I${SRC} -I${SRC}/vendor/cnpy -o ${ENERGY_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}


ENERGY_MOLECULES = 10000
ENERGY_STEPS = 1000
DISLAY_EVERY = 1
energy : compile_energy
	${ENERGY_EXE} ${ENERGY_MOLECULES} ${ENERGY_STEPS} ${DISLAY_EVERY}

######### Iso Processes #########

PROCESS_EXE = experiments/isoproc/isoproc
PROCESS_SRC = experiments/isoproc/isoproc.cpp

compile_iso: ${PROCESS_SRC} ${SRC}/bin/libmodel.so
	g++ ${CCFLAGS} ${PROCESS_SRC} -I${SRC} -I${SRC}/vendor/cnpy -o ${PROCESS_EXE} ${LINK_TO_MODEL} ${LINK_TO_CNPY_FLAGS}

#==================================================================================================
# VISUALIZATION
#==================================================================================================

RES_COORDS     = res/coords.npy
RES_VELOCITIES = res/velocities.npy
RES_TYPES      = res/types.npy

VISUALIZE_SCRIPT = visualization/mol_vis.py

RENDER_MODE = --cubesize 500 --realtime 1

visualize : ${RES_COORDS} ${RES_VELOCITIES} ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} ${RES_COORDS} ${RES_VELOCITIES}

demo_waves : res/WAVES_coords.npy res/WAVES_velocities.npy ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} res/WAVES_coords.npy res/WAVES_velocities.npy

#==================================================================================================
# MODEL OPTIMIZATION
#==================================================================================================

MODEL_ASM = optimization/listing.asm
OPTIMIZE_HDRS = model/Model.hpp model/Model_SSE.hpp model/Model_BARNES_HUT.hpp model/Model_HARDCORE.hpp

compile_debug : ${MODEL_SRC} ${MODEL_HDRS}
	g++ ${MODEL_SRC} -S -o ${MODEL_ASM} -g ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}
	g++ ${MODEL_SRC}    -o ${MODEL_EXE} -g ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}

profile : compile_debug
	rm -f ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}

#==================================================================================================
# MISCELLANEOUS
#==================================================================================================

directories:
	mkdir -p model/bin

clean:
	rm -rf model/bin
