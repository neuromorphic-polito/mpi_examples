project(mpi_nbody)
cmake_minimum_required(VERSION 3.6)
set(CMAKE_C_STANDARD 99)

# --->
message("[CMAKE] Init")

set(SPINN_DIRS $ENV{SPINN_DIRS})

set(LIBRARIES
        -lspinn_mpi
        -lspinn_acp
        -lspinn_two
        -lspin1_api
        -lsark
        )

set(APP mpi_nbody)
set(APP_SRC ${CMAKE_SOURCE_DIR}/src/${APP}.c)

set(APP_INC_DIR
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/include
        ${CMAKE_SOURCE_DIR}/lib/nbody/include
)
set(APP_SRC_OTHERS
        # LIBFIXMATH
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/source/fix16.c
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/source/fix16_exp.c
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/source/fix16_sqrt.c
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/source/fix16_str.c
        ${CMAKE_SOURCE_DIR}/lib/libfixmath/source/fix16_trig.c
        # NBODY
        ${CMAKE_SOURCE_DIR}/lib/nbody/source/nbody_core.c
        ${CMAKE_SOURCE_DIR}/lib/nbody/source/nbody_init.c
        ${CMAKE_SOURCE_DIR}/lib/nbody/source/nbody_utils.c
)

include(CMakeListsApp.cmake)

message("[CMAKE] End")
# <---
