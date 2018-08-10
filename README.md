# hs2client: C++ Thrift client for Impala and Hive

<table>
  <tr>
    <td>Build Status</td>
    <td>
    <a href="https://travis-ci.org/cloudera/hs2client">
    <img src="https://travis-ci.org/cloudera/hs2client.svg?branch=master" alt="travis build status" />
    </a>
    </td>
  </tr>
</table>

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

# How do I contribute code?
You need to first sign and return an
[ICLA](https://github.com/cloudera/native-toolchain/blob/icla/Cloudera%20ICLA_25APR2018.pdf)
and
[CCLA](https://github.com/cloudera/native-toolchain/blob/icla/Cloudera%20CCLA_25APR2018.pdf)
before we can accept and redistribute your contribution. Once these are submitted you are
free to start contributing to hs2client. Submit these to CLA@cloudera.com.

## Find
We use Github issues to track bugs for this project. Find an issue that you would like to
work on (or file one if you have discovered a new issue!). If no-one is working on it,
assign it to yourself only if you intend to work on it shortly.

It’s a good idea to discuss your intended approach on the issue. You are much more
likely to have your patch reviewed and committed if you’ve already got buy-in from the
hs2client community before you start.

## Fix
Now start coding! As you are writing your patch, please keep the following things in mind:

First, please include tests with your patch. If your patch adds a feature or fixes a bug
and does not include tests, it will generally not be accepted. If you are unsure how to
write tests for a particular component, please ask on the issue for guidance.

Second, please keep your patch narrowly targeted to the problem described by the issue.
It’s better for everyone if we maintain discipline about the scope of each patch. In
general, if you find a bug while working on a specific feature, file a issue for the bug,
check if you can assign it to yourself and fix it independently of the feature. This helps
us to differentiate between bug fixes and features and allows us to build stable
maintenance releases.

Finally, please write a good, clear commit message, with a short, descriptive title and
a message that is exactly long enough to explain what the problem was, and how it was
fixed.

Please post your patch to the hs2client project at https://gerrit.cloudera.org
for review. See
[Impala's guide on using gerrit](https://cwiki.apache.org/confluence/display/IMPALA/Using+Gerrit+to+submit+and+review+patches)
to submit and review patches for instructions on how to send patches to
http://gerrit.cloudera.org, except make sure to send your patch to the hs2client
project instead of Impala-ASF.
