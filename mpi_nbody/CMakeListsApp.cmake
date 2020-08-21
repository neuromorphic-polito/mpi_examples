message("[CMAKE-APP] Init")

if(NOT SPINN_DIRS)
    message(FATAL_ERROR "SPINN_DIRS is not set!")
endif()

if(NOT APP)
    message(FATAL_ERROR "APP is not set!")
endif()

set(SPINN_INC_DIR ${APP_INC_DIR})

include(CMakeListsCommon.cmake)

add_custom_target(
        spinn_all ALL
        DEPENDS ${APP}.aplx
)

add_custom_command(
        TARGET spinn_all
        POST_BUILD
        COMMAND cp ${APP}.aplx ${CMAKE_SOURCE_DIR}/${PROJECT_NAME}.aplx
        COMMAND ${RM} ${OBJECTS} ${APP}.txt ${APP}.elf
        #COMMAND ${RM} ${APP}.aplx
)

message("[CMAKE-APP] End")
