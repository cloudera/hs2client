# Copyright 2012 Cloudera Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
# http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 2.6)

# Helper function to generate build rules.  For each input thrift file, this function will
# generate a rule that maps the input file to the output c++ file.
# Thrift will generate multiple output files for each input (including java files) and
# ideally, we'd specify all of the outputs for dependency tracking.
# Unfortunately, it's not easy to figure out all the output files without parsing the
# thrift input. (TODO: can thrift tells us what the java output files will be?)
# The list of output files is used for build dependency tracking so it's not necessary to
# capture all the output files.
#
# To call this function, pass it the output file list followed by the input thrift files:
#    i.e. THRIFT_GEN(OUTPUT_FILES, ${THRIFT_FILES})
#
# cmake seems to be case sensitive for some keywords. Changing the first IF check to lower
# case makes it not work.  TODO: investigate this
function(THRIFT_GEN VAR)
  IF (NOT ARGN)
    MESSAGE(SEND_ERROR "Error: THRIFT_GEN called without any src files")
    RETURN()
  ENDIF(NOT ARGN)

  set(${VAR})
  foreach(FIL ${ARGN})
    # Get full path
    get_filename_component(ABS_FIL ${FIL} ABSOLUTE)
    # Get basename
    get_filename_component(FIL_WE ${FIL} NAME_WE)

    # All the output files we can determine based on filename.
    #   - Does not include .skeleton.cpp files
    #   - Does not include java output files
    set(OUTPUT_BE_FILE "${OUTPUT_DIR}/gen-cpp/${FIL_WE}_types.cpp")
    set(OUTPUT_BE_FILE ${OUTPUT_BE_FILE} " ${OUTPUT_DIR}/gen-cpp/${FIL_WE}_types.h")
    set(OUTPUT_BE_FILE ${OUTPUT_BE_FILE} " ${OUTPUT_DIR}/gen-cpp/${FIL_WE}_constants.cpp")
    set(OUTPUT_BE_FILE ${OUTPUT_BE_FILE} " ${OUTPUT_DIR}/gen-cpp/${FIL_WE}_constants.h")
    list(APPEND ${VAR} ${OUTPUT_BE_FILE})

    # BeeswaxService thrift generation
    # It depends on hive_meta_store, which in turn depends on fb303.
    # The java dependency is handled by maven.
    # We need to generate C++ src file for the parent dependencies using the "-r" option.
    set(CPP_ARGS --gen cpp -o ${OUTPUT_DIR})
    IF (FIL STREQUAL "beeswax.thrift")
      set(CPP_ARGS -r --gen cpp -o ${OUTPUT_DIR})
    ENDIF(FIL STREQUAL "beeswax.thrift")

    add_custom_command(
      OUTPUT ${OUTPUT_BE_FILE}
      COMMAND ${THRIFT_COMPILER} ${CPP_ARGS} ${FIL}
      DEPENDS ${ABS_FIL}
      COMMENT "Running thrift compiler on ${FIL}"
	  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
      VERBATIM
    )
  endforeach(FIL)

  set(${VAR} ${${VAR}} PARENT_SCOPE)
endfunction(THRIFT_GEN)

message("Using Thrift compiler: ${THRIFT_COMPILER}")

set(OUTPUT_DIR ${CMAKE_SOURCE_DIR}/src)
file(MAKE_DIRECTORY ${OUTPUT_DIR})

set(SRC_FILES
  ErrorCodes.thrift
  beeswax.thrift
  TCLIService.thrift
  ExecStats.thrift
  ImpalaService.thrift
  Status.thrift
  Types.thrift
)

SET_SOURCE_FILES_PROPERTIES(Status.thrift PROPERTIES OBJECT_DEPENDS ErrorCodes.thrift)

add_custom_command(OUTPUT ErrorCodes.thrift
  COMMAND python generate_error_codes.py
  DEPENDS generate_error_codes.py
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})

# Create a build command for each of the thrift src files and generate
# a list of files they produce
THRIFT_GEN(THRIFT_ALL_FILES ${SRC_FILES})

add_custom_target(thrift-generated-files-error DEPENDS ErrorCodes.thrift)

# Add a custom target that generates all the thrift files
add_custom_target(thrift-cpp ALL DEPENDS ${THRIFT_ALL_FILES})
add_dependencies(thrift-cpp thrift-generated-files-error)
