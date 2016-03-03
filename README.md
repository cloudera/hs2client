# hs2client: C++ Thrift client for Impala and Hive

## Development bootstrap

```bash
# Build thirdparty Thrift and googletest and set $THRIFT_HOME.

./thirdparty/download_thirdparty.sh
./thirdparty/build_thirdparty.sh
source thirdparty/set_thirdparty_env.sh
```

If you are using cloudera/native-toolchain, you can instead use an environment
setup script like:

```bash
export BOOST_VERSION=1.57.0
export THRIFT_VERSION=0.9.0-p2
export GTEST_VERSION=20151222

if [[ $OSTYPE == "darwin"* ]]; then
	THRIFT_VERSION=0.9.2-p2
fi

export CC=$NATIVE_TOOLCHAIN/gcc-4.9.2/bin/gcc
export CXX=$NATIVE_TOOLCHAIN/gcc-4.9.2/bin/g++
export LD_LIBRARY_PATH=$NATIVE_TOOLCHAIN/gcc-4.9.2/lib64

export THRIFT_HOME=$NATIVE_TOOLCHAIN/thrift-$THRIFT_VERSION
export BOOST_HOME=$NATIVE_TOOLCHAIN/boost-$BOOST_VERSION
export GTEST_HOME=$NATIVE_TOOLCHAIN/googletest-$GTEST_VERSION
```

This is configured for out-of-source builds:

```bash
mkdir build-debug
cd build-debug
cmake ..
make -j4
```