#!/bin/bash

set -e
set -x

cd $RECIPE_DIR

# Build dependencies
export BOOST_ROOT=$PREFIX
export THRIFT_HOME=$PREFIX

cd ..

# Build googletest for running unit tests
./thirdparty/download_thirdparty.sh
./thirdparty/build_thirdparty.sh gtest

source thirdparty/versions.sh
export GTEST_HOME=`pwd`/thirdparty/$GTEST_BASEDIR

if [ `uname` == Linux ]; then
    SHARED_LINKER_FLAGS='-static-libstdc++'
elif [ `uname` == Darwin ]; then
    SHARED_LINKER_FLAGS=''
fi

# HACK: build in-place until we figure out out of source builds
rm -f CMakeCache.txt

cmake \
    -DCMAKE_BUILD_TYPE=release \
    -DCMAKE_INSTALL_PREFIX=$PREFIX \
    -DCMAKE_SHARED_LINKER_FLAGS=$SHARED_LINKER_FLAGS \
    -DHS2CLIENT_BUILD_TESTS=off \
    .

make
make install
