M<sup>3</sup> for Batch Processing
====

M<sup>3</sup> for Batch Processing (M<sup>3</sup> for BP) is a data processing engine that fully extracts the performance of multicore/multiprocessor-based single node servers.

M<sup>3</sup> for BP is designed for [Asakusa Framework](https://github.com/asakusafw/asakusafw), with a focus on small-to-medium volume of data (less than tens of GB).  For this volume of data, M<sup>3</sup> for BP provides much better performance and much lower TCO for applications built on Asaskusa Framework than do MapReduce and Spark.

## Prerequisites
- CMake (>= 2.8)
- Boost C++ libraries (>= 1.55)
- hwloc (>= 1.8)

## How to build and install
```
$ git submodule init
$ git submodule update
$ mkdir build && cd build
$ cmake -DCMAKE_BUILD_TYPE=Release [-DCMAKE_INSTALL_PREFIX=/path/to/install] .. 
$ make
$ make install
```

## License
- [Apache License, Version 2.0](http://www.apache.org/licenses/LICENSE-2.0)

