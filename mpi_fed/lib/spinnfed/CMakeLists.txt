# ===========================================================================
# include/CMakeLists.txt
# ----------------------
#
# Module for creating the FED library core target.
# ---------------------------------------------------------------------------
#
# author : Emanuele Parisi
# ===========================================================================

# ---------------------------------------------------------------------------
# Configure the library global target FED.
# ---------------------------------------------------------------------------

set_source_files_properties(src/fed_utils.c
                            PROPERTIES
                            LANGUAGE CXX)
set_source_files_properties(src/fed_text.c
                            PROPERTIES
                            LANGUAGE CXX)
set_source_files_properties(src/fed_pattern.c
                            PROPERTIES
                            LANGUAGE CXX)
set_source_files_properties(src/fed_debug.c
                            PROPERTIES
                            LANGUAGE CXX)
set_source_files_properties(src/fed_search.c
                            PROPERTIES
                            LANGUAGE CXX)
add_library(SpiNNFED
            src/fed_utils.c
            src/fed_text.c
            src/fed_pattern.c
            src/fed_debug.c
            src/fed_search.c)
target_include_directories(SpiNNFED
                           PUBLIC
                           ${CMAKE_SOURCE_DIR})
target_compile_definitions(SpiNNFED
                           PUBLIC
                           -DSPINNFED_VERSION="${SpiNNFED_VERSION}")
option(SpiNNFED_ENABLE_DBG_FUNCS
       "Choose if debug prints should be enable or not" OFF)
if (${SpiNNFED_ENABLE_DBG_FUNCS})
	target_compile_definitions(SpiNNFED
	                           PUBLIC
	                           -DSPINNFED_ENABLE_DBG_FUNCS=1)
endif ()
