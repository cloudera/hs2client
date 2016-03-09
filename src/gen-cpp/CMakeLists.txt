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

set(THRIFT_SRC
  ImpalaService.cpp
  ImpalaService_constants.cpp
  ImpalaService_types.cpp
  ImpalaHiveServer2Service.cpp
  beeswax_constants.cpp
  beeswax_types.cpp
  BeeswaxService.cpp
  TCLIService.cpp
  TCLIService_constants.cpp
  TCLIService_types.cpp
  ExecStats_constants.cpp
  ExecStats_types.cpp
  hive_metastore_constants.cpp
  hive_metastore_types.cpp
  Status_constants.cpp
  Status_types.cpp
  Types_constants.cpp
  Types_types.cpp
)

set_source_files_properties(${THRIFT_SRC} PROPERTIES
  COMPILE_FLAGS -Wno-unused-variable)

# keep everything in one library, the object files reference
# each other
add_library(hs2client_thrift ${THRIFT_SRC})

# Setting these files as code-generated lets make clean and incremental builds work
# correctly

# TODO(wesm): Something is broken with the dependency chain with
# ImpalaService.cpp and others. Couldn't figure out what is different between
# this setup and Impala.

set_source_files_properties(${THRIFT_SRC} PROPERTIES GENERATED TRUE)

add_dependencies(hs2client_thrift thrift-cpp)

set_target_properties(hs2client_thrift
  PROPERTIES
  LIBRARY_OUTPUT_DIRECTORY "${BUILD_OUTPUT_ROOT_DIRECTORY}")
