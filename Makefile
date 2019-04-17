#==================================================================================================
# COMPILER FLAGS
#==================================================================================================

COMPILER = g++
CCFLAGS += -std=c++17 -Werror -Wall -fno-stack-protector

#==================================================================================================
# VISUALIZATION
#==================================================================================================

RES_COORDS     = res/coords.npy
RES_VELOCITIES = res/velocities.npy

VISUALIZE_SCRIPT = visualization/mol_vis.py

visualize : ${RES_COORDS} ${RES_VELOCITIES} ${VISUALIZE_SCRIPT}
	python3 ${VISUALIZE_SCRIPT} --cubesize 500 --realtime 1 ${RES_COORDS} ${RES_VELOCITIES}

#==================================================================================================
# MODEL
#==================================================================================================

MODEL_EXE  = model/bin/model
MODEL_SRC  = model/model.cpp
MODEL_HDRS = model/Model.hpp model/SavingToFile.hpp

LINK_TO_CNPY_FLAGS = -L/usr/local -lcnpy -lz

compile : ${MODEL_EXE}
 
${MODEL_EXE} : ${MODEL_SRC} ${CNPY_OBJ} ${MODEL_HDRS}
	g++ ${MODEL_SRC} -o ${MODEL_EXE} ${LINK_TO_CNPY_FLAGS} --std=c++17

model : ${MODEL_EXE}
	rm -f ${RES_COORDS} ${RES_VELOCITIES}
	${MODEL_EXE} ${RES_COORDS} ${RES_VELOCITIES}

#==================================================================================================
# MICELLANEOUS
#==================================================================================================

directories:
	mkdir -p model/bin

clean:
	rm -rf model/bin

