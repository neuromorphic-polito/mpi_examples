message("[CMAKE-COMMON] Init")

if(NOT SPINN_DIRS)
    message(FATAL_ERROR "SPINN_DIRS is not set!")
endif()

set(THUMB 0)
set(DEBUG 0)
set(LIB 0)

set(SPINN_INC_DIR ${SPINN_DIRS}/include ${SPINN_INC_DIR})
set(SPINN_LIB_DIR ${SPINN_DIRS}/lib)
set(SPINN_LIBS
        ${SPINN_LIB_DIR}/libsark.a
        ${SPINN_LIB_DIR}/libspin1_api.a
        ${SPINN_LIB_DIR}/libspinn_mpi.a
        ${SPINN_LIB_DIR}/libspinn_acp.a)

set(SPINN_TOOLS_DIR ${SPINN_DIRS}/tools)

message("APP sources:")
foreach(loop_var IN LISTS APP_SRC)
    message(\t ${loop_var})
endforeach(loop_var)

message("APP other sources:")
foreach(loop_var IN LISTS APP_SRC_OTHERS)
    message(\t ${loop_var})
endforeach(loop_var)

message("Include dirs:")
foreach(loop_var IN LISTS SPINN_INC_DIR)
    message(\t ${loop_var})
    set(CC_INCLUDE ${CC_INCLUDE} -I ${loop_var})
endforeach(loop_var)

# --> Commands
set(RM rm -f)
set(RMDIR rmdir)
set(CAT cat)
set(LS ls -l)
set(MKDIR mkdir -p)
set(CP cp)
set(MV mv)

# --> GNU Compiler
set(GP arm-none-eabi)
set(OSPACE -Os)
set(OTIME -Ofast)

set(AR ${GP}-ar -rcs)
set(OC ${GP}-objcopy)
set(OD ${GP}-objdump -dxt >)
set(NM ${GP}-nm)

set(AS
        ${GP}-as
        --defsym GNU=1
        -mthumb-interwork
        -march=armv5te)

set(CC_NO_THUMB
        ${GP}-gcc
        -c
        #-mthumb-interwork
        -march=armv5te
        -std=c99
        ${CC_INCLUDE})

set(CXX_NO_THUMB
        ${GP}-gcc
        -c
        #-mthumb-interwork
        -march=armv5te
        -std=c++11
        ${CC_INCLUDE}
        -fno-rtti
        -fno-exceptions)

set(CC_THUMB
        ${CC_NO_THUMB}
        -mthumb
        -DTHUMB)

set(CXX_THUMB
        ${CXX_NO_THUMB}
        -mthumb
        -DTHUMB)

if(LIB EQUAL 1)
    set(CFLAGS ${CFLAGS} -fdata-sections -ffunction-sections)
    set(LD ${GP}-ld -i)
else()
    set(LFLAGS ${LFLAGS} -L ${SPINN_LIB_DIR})
    set(LD
            ${GP}-gcc
            -T${SPINN_TOOLS_DIR}/sark.lnk
            -Wl,-e,cpu_reset
            -Wl,-static
            -Wl,--gc-sections
            -Wl,--use-blx
            -fdata-sections
            -ffunction-sections
            -nostartfiles
            -static
            )
endif()
message("${LD}")

if(THUMB EQUAL 1)
    set(CC ${CC_THUMB})
    set(CXX "${CXX_THUMB}")
else()
    set(CC ${CC_NO_THUMB})
    set(CXX "${CXX_NO_THUMB}")
endif()
message("${CC}")


if(DEBUG EQUAL 1)
    set(CFLAGS ${CFLAGS} -g)
    set(AFLAGS ${AFLAGS} -g)
endif()

set(OBJECTS)
set(OBJECTS_BUILD)

message("Create commands for app:")
foreach(loop_var_path IN LISTS APP_SRC)
    get_filename_component(loop_var_src ${loop_var_path} NAME)
    get_filename_component(loop_var_dir ${loop_var_path} DIRECTORY)

    STRING(REGEX REPLACE "\\.c" "_build.c" loop_var_src_build ${loop_var_src})
    message(\t${loop_var_dir}/${loop_var_src} -> ${loop_var_src_build} )
    add_custom_command(
            OUTPUT ${loop_var_src_build}
            COMMAND ${SPINN_TOOLS_DIR}/mkbuild ${loop_var_dir}/${loop_var_src} > ${loop_var_src_build}
            DEPENDS ${loop_var_dir}/${loop_var_src}
    )

    STRING(REGEX REPLACE "\\.c" ".o" loop_var_dst ${loop_var_src})
    message(\t${loop_var_dir}/${loop_var_src} -> ${loop_var_dst} )
    add_custom_command(
            OUTPUT ${loop_var_dst}
            COMMAND ${CC} ${CFLAGS} -o ${loop_var_dst} ${loop_var_dir}/${loop_var_src}
            DEPENDS ${loop_var_dir}/${loop_var_src}
    )
    set(OBJECTS ${OBJECTS} ${loop_var_dst})

    STRING(REGEX REPLACE "\\.c" ".o" loop_var_dst ${loop_var_src_build})
    message(\t${loop_var_src_build} -> ${loop_var_dst} )
    add_custom_command(
            OUTPUT ${loop_var_dst}
            COMMAND ${CC} ${CFLAGS} -o ${loop_var_dst}  ${loop_var_src_build}
            DEPENDS ${loop_var_src_build}
    )
    set(OBJECTS_BUILD ${OBJECTS_BUILD} ${loop_var_dst})
endforeach(loop_var_path)

message("Create commands for lib:")
foreach(loop_var_path IN LISTS APP_SRC_OTHERS)
    get_filename_component(loop_var_src ${loop_var_path} NAME)
    get_filename_component(loop_var_dir ${loop_var_path} DIRECTORY)

    STRING(REGEX REPLACE "\\.c" "_build.c" loop_var_src_build ${loop_var_src})
    message(\t${loop_var_dir}/${loop_var_src} -> ${loop_var_src_build} )
    add_custom_command(
            OUTPUT ${loop_var_src_build}
            COMMAND ${SPINN_TOOLS_DIR}/mkbuild ${loop_var_dir}/${loop_var_src} > ${loop_var_src_build}
            DEPENDS ${loop_var_dir}/${loop_var_src}
    )

    STRING(REGEX REPLACE "\\.c" ".o" loop_var_dst ${loop_var_src})
    message(\t${loop_var_dir}/${loop_var_src} -> ${loop_var_dst} )
    add_custom_command(
            OUTPUT ${loop_var_dst}
            COMMAND ${CC} ${CFLAGS} -o ${loop_var_dst} ${loop_var_dir}/${loop_var_src}
            DEPENDS ${loop_var_dir}/${loop_var_src}
    )
    set(OBJECTS ${OBJECTS} ${loop_var_dst})

    STRING(REGEX REPLACE "\\.c" ".o" loop_var_dst ${loop_var_src_build})
    message(\t${loop_var_src_build} -> ${loop_var_dst} )
    add_custom_command(
            OUTPUT ${loop_var_dst}
            COMMAND ${CC} ${CFLAGS} -o ${loop_var_dst}  ${loop_var_src_build}
            DEPENDS ${loop_var_src_build}
    )
    set(OBJECTS_BUILD ${OBJECTS_BUILD} ${loop_var_dst})
endforeach(loop_var_path)

message("OBJECTS: ${OBJECTS}")
message("OBJECTS_BUILD: ${OBJECTS_BUILD}")
message("LIBRARIES: ${LIBRARIES}")
message("SPINN_LIBS: ${SPINN_LIBS}")
message("LFLAGS: ${LFLAGS}")

# Build the ELF file
#  1) Link application object(s), build file and library to make the ELF
#  2) Create a list file
add_custom_command(
        OUTPUT ${APP}.elf
        COMMAND ${LD} ${LFLAGS} ${OBJECTS} ${APP}_build.o ${LIBRARIES} ${SPINN_LIBS} -o ${APP}.elf
        COMMAND ${OD} ${APP}.txt ${APP}.elf
        DEPENDS ${OBJECTS} ${OBJECTS_BUILD}
)

# Create a binary file which is the concatenation of RO and RW sections
add_custom_command(
        OUTPUT ${APP}.bin
        COMMAND ${OC} -O binary -j RO_DATA ${APP}.elf RO_DATA.bin
        COMMAND ${OC} -O binary -j RW_DATA ${APP}.elf RW_DATA.bin
        COMMAND ${SPINN_TOOLS_DIR}/mkbin RO_DATA.bin RW_DATA.bin > ${APP}.bin
        COMMAND ${RM} RO_DATA.bin RW_DATA.bin
        DEPENDS ${APP}.elf
)

# Create a list of the objects in the file using nm
add_custom_command(
        OUTPUT ${APP}.nm
        COMMAND ${NM} ${APP}.elf > ${APP}.nm
        DEPENDS ${APP}.elf
)

# Primary target is an APLX file - built from the ELF
#  1) Make an APLX header from the ELF file with "mkaplx" and concatenate
#     that with the binary to make the APLX file
#  2) "ls" the APLX file
add_custom_command(
        OUTPUT ${APP}.aplx
        COMMAND ${SPINN_TOOLS_DIR}/mkaplx ${APP}.nm | ${CAT} - ${APP}.bin > ${APP}.aplx
        DEPENDS ${APP}.bin ${APP}.nm
)

message("[CMAKE-COMMON] End")
