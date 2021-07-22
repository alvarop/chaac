cmake_minimum_required(VERSION 3.18)
project(chaac
	LANGUAGES C CXX ASM)

set_property(GLOBAL PROPERTY C_STANDARD 11)
set_property(GLOBAL PROPERTY CXX_STANDARD 17)

include(cmake/CPM.cmake)

if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME OR LIBC_BUILD_TESTING)
    include(CTest)
endif()

include(cmake/AddExecutableWithLinkerScriptDep.cmake)
include(cmake/CheckAndApplyFlags.cmake)

include(BuildOptions.cmake)

##################
# Compiler Flags #
##################

# If we're not a subproject, globally apply our warning flags
if(CMAKE_PROJECT_NAME STREQUAL PROJECT_NAME)
	add_compile_options(-Wall -Wextra)
endif()

# This function can be used to generate a linker map file on an executable target
# in a compiler-agnostic way (currently supports GCC and Clang compiler families)
function(target_linker_map target)
  check_c_linker_flag("-Wl,-Map,test.map" _Map1)
  if("${_Map1}")
    set(GEN_MAP_FILE "-Wl,-Map,")
  else()
    check_c_linker_flag("-Wl,-map,test.map" _Map2)
    if("${_Map2}")
      set(GEN_MAP_FILE "-Wl,-map,")
    else()
      message("[WARNING] ${CMAKE_C_COMPILER_ID} does not have a defined linker map argument.")
      return()
    endif()
  endif()

  get_target_property(map_dir ${target} BINARY_DIR)
  target_link_options(${target} PRIVATE ${GEN_MAP_FILE}${map_dir}/${target}.map)
  set_target_properties(${target} PROPERTIES
    ADDITIONAL_CLEAN_FILES ${target}.map)
endfunction()


#######################
# Process Source Tree #
#######################

add_subdirectory(src)

#####################
# Process Test Tree #
#####################

# TODO - add tests
# add_subdirectory(test)


