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
        ${CMAKE_SOURCE_DIR}/lib/template_lib1/include
        ${CMAKE_SOURCE_DIR}/lib/template_lib2/include
        )
set(APP_SRC_OTHERS
        # TEMPLATE_LIB1 list all source file
        ${CMAKE_SOURCE_DIR}/lib/template_lib1/source/lib1.c
        # TEMPLATE_LIB2 list all source file
        ${CMAKE_SOURCE_DIR}/lib/template_lib2/source/lib2.c
        )

include(CMakeListsApp.cmake)

message("[CMAKE] End")
# <---
