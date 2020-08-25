project(mpi_template)
cmake_minimum_required(VERSION 3.5)
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

set(APP template)
set(APP_SRC ${CMAKE_SOURCE_DIR}/src/${APP}.c)

set(APP_INC_DIR
        ${CMAKE_SOURCE_DIR}/lib/spinnfed/include
        ${CMAKE_SOURCE_DIR}/include
        )
set(APP_SRC_OTHERS
        #${CMAKE_SOURCE_DIR}/lib/spinnfed/src/fed_debug.c
        ${CMAKE_SOURCE_DIR}/lib/spinnfed/src/fed_pattern.c
        ${CMAKE_SOURCE_DIR}/lib/spinnfed/src/fed_search.c
        ${CMAKE_SOURCE_DIR}/lib/spinnfed/src/fed_text.c
        ${CMAKE_SOURCE_DIR}/lib/spinnfed/src/fed_utils.c
        )

include(CMakeListsApp.cmake)

message("[CMAKE] End")
# <---
