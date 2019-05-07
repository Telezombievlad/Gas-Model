.DEFAULT_GOAL := default

#==================================================================================================
# COMPILER FLAGS
#==================================================================================================

COMPILER = g++
CCFLAGS += -std=c++17 -Werror -Wall -O3 -mavx -fno-stack-protector

#==================================================================================================
# LIBRARY INSTALLATION
#==================================================================================================

SRC = model

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


${SRC}/bin/model.o : ${SRC}/Model.h ${SRC}/Model.cpp
	g++ -fPIC -c ${SRC}/Model.cpp -o ${SRC}/bin/model.o ${CCFLAGS}

${SRC}/bin/dimensioning.o : ${SRC}/Dimensioning.h ${SRC}/Dimensioning.cpp
	g++ -fPIC -c ${SRC}/Dimensioning.cpp -o ${SRC}/bin/dimensioning.o ${CCFLAGS}

${SRC}/bin/save.o : ${SRC}/SavingToFile.h ${SRC}/SavingToFile.cpp
	g++ -fPIC -c ${SRC}/SavingToFile.cpp -o ${SRC}/bin/save.o ${CCFLAGS}

${SRC}/bin/libmodel.so : ${SRC}/bin/model.o ${SRC}/bin/dimensioning.o ${SRC}/bin/save.o
	g++ -fPIC -shared -o ${SRC}/bin/libmodel.so ${SRC}/bin/model.o ${SRC}/bin/dimensioning.o ${SRC}/bin/save.o

default: ${SRC}/bin/libmodel.so
	@ echo "Library compiled."

#set -x LD_LIBRARY_PATH /home/max/Documents/MIPT/GasSim/Gas-Model/model/bin $LD_LIBRARY_PATH

#==================================================================================================
# EXPERIMENTS
#==================================================================================================

######### Static modeling #########

MODEL_EXE  = experiments/modeling/model
MODEL_SRC  = experiments/modeling/model.cpp
LINK_TO_CNPY_FLAGS = -L/usr/local -lcnpy -lz

compile_model : ${MODEL_SRC} ${SRC}/bin/libmodel.so
	g++ ${MODEL_SRC} -I${SRC} -L${SRC}/bin -lmodel -o ${MODEL_EXE} ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}

model : compile_model
	rm -f ${RES_COORDS} ${RES_VELOCITIES}
	${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES}

######### Energy conservation #########

ENERGY_EXE = experiments/energy/en
ENERGY_SRC = experiments/energy/energy.cpp

compile_energy : ${ENERGY_SRC} ${SRC}/bin/libmodel.so
	g++ ${ENERGY_SRC} -I${SRC} -I${SRC}/vendor/cnpy -L${SRC}/bin -lmodel -o ${ENERGY_EXE} ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}

#==================================================================================================
# VISUALIZATION
#==================================================================================================

RES_COORDS     = res/coords.npy
RES_VELOCITIES = res/velocities.npy

VISUALIZE_SCRIPT = visualization/mol_vis.py

RENDER_MODE = --cubesize 500 --realtime 1

visualize : ${RES_COORDS} ${RES_VELOCITIES} ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} ${RES_COORDS} ${RES_VELOCITIES}

demo_waves : res/WAVES_coords.npy res/WAVES_velocities.npy ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} res/WAVES_coords.npy res/WAVES_velocities.npy

demo_press : res/PRESS_coords.npy res/PRESS_velocities.npy ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} res/PRESS_coords.npy res/PRESS_velocities.npy

#==================================================================================================
# MODEL OPTIMIZATION
#==================================================================================================

MODEL_ASM = optimization/listing.asm
OPTIMIZE_HDRS = model/Model.hpp model/Model_SSE.hpp model/Model_BARNES_HUT.hpp model/Model_HARDCORE.hpp

compile_debug : ${MODEL_SRC} ${MODEL_HDRS}
	g++ ${MODEL_SRC} -S -o ${MODEL_ASM} -g ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}
	g++ ${MODEL_SRC}    -o ${MODEL_EXE} -g ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}

profile : compile_debug
	rm -f ${RES_COORDS} ${RES_VELOCITIES}
	valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES}

#==================================================================================================
# MISCELLANEOUS
#==================================================================================================

directories:
	mkdir -p model/bin

clean:
	rm -rf model/bin
