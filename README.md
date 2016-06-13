# hs2client: C++ Thrift client for Impala and Hive

This is a new project to create a robust, high performance open source
HiveServer2 client library in C++. In addition to being a useful component for
other C++ projects needing to communicate with Hive, Impala, and any systems
using the HiveServer2 protocol, we can also create CPU- and memory-efficient
bindings for other languages, like Python and R.

There is work still remaining to bring the project to feature-completeness with
other Hive and Impala driver libraries:

- SSL support
- SASL Thrift transport (for secure clusters, or unsecure clusters configured
  to use the SASL transport)
- Tools for asynchronous result set fetching

## Development bootstrap

To build standalone thirdparty dependencies, ensure you are using a C++11
compiler and run:

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

Currently, only in-source builds are working:

```bash
cmake .
make -j4
```