#==================================================================================================
# COMPILER FLAGS
#==================================================================================================

CCFLAGS += -std=c++17 -Werror -Wall -O3 -mavx -fno-stack-protector

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

demo_press : res/PRESS_coords.npy res/PRESS_velocities.npy ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} ${RENDER_MODE} res/PRESS_coords.npy res/PRESS_velocities.npy

#==================================================================================================
# MODEL
#==================================================================================================

MODEL_EXE  = model/bin/model
MODEL_SRC  = model/model.cpp
MODEL_HDRS = model/Model.hpp model/SavingToFile.hpp

LINK_TO_CNPY_FLAGS = -L/usr/local -lcnpy -lz

compile : ${MODEL_SRC} ${MODEL_HDRS}
	g++ ${MODEL_SRC} -o ${MODEL_EXE} ${CCFLAGS} ${LINK_TO_CNPY_FLAGS}

model : compile
	rm -f ${RES_COORDS} ${RES_VELOCITIES}
	${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES} ${RES_TYPES}

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
# MICELLANEOUS
#==================================================================================================

directories:
	mkdir -p model/bin

clean:
	rm -rf model/bin

