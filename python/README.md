## Python interface to hs2client HiveServer 2 library

To build, you must first build and install the hs2client C++ library. Set
`$HS2CLIENT_HOME` to the installation directory, then run:

```shell
python setup.py install
```

Installation requires

- A compliant C++11 compiler (gcc 4.8, clang 3.4, VS 2015, or higher)
- Cython
- numpy

### Roadmap items

- Python DB API 2.0 support
