#!/bin/bash
set -ex

# Build dependency
export HS2CLIENT_HOME=$PREFIX

cd $RECIPE_DIR

# For some reason the conda-forge toolchain does not work properly here
if [ "$(uname)" == "Darwin" ]; then
    export MACOSX_DEPLOYMENT_TARGET=10.7
    export CFLAGS="${CXXFLAGS} -stdlib=libc++ -std=c++11"
    export CXXFLAGS="${CXXFLAGS} -stdlib=libc++ -std=c++11"
fi

pwd

cd ..
$PYTHON setup.py install --single-version-externally-managed --record=record.txt
