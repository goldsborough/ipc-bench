# IPC-Bench

[![GitHub license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square)](http://goldsborough.mit-license.org)

This repository holds sample implementations and benchmarks for various inter-process-communication (IPC) methods on Linux and OS X.

## Spectrum

Currently, the following IPC methods are implemented and ready to be benchmarked:

* Internet sockets
* Domain sockets
* Pipes
* FIFOs (named pipes)
* Shared Memory
* Memory-Mapped Files
* Message Queues
* ZeroMQ
* Unix Signals

Our benchmarks measure the latency of sending a single message between two processes (see below).

## Usage

Some required packages on Ubuntu:
```shell
sudo apt-get install pkg-config
```

You can build the project and all necessary executables using CMake. The following commands (executed from the root folder) should do the trick:

```shell
mkdir build
cd build
cmake ..
make
```

This will generate a `build/source` folder, holding further directories for each IPC type. Simply execute the program named after the folder, e.g. `build/source/shm/shm`. Where applicable, this will start a new server and client process, run benchmarks and print results to `stdout`. For example, running `build/source/shm/shm` outputs:

```
============ RESULTS ================
Message size:       4096
Message count:      1000
Total duration:     1.945      	ms
Average duration:   1.418      	us
Minimum duration:   0.000      	us
Maximum duration:   25.000     	us
Standard deviation: 1.282      	us
Message rate:       514138     	msg/s
=====================================
```

The benchmarks measure the latency of sending a single message from the client to the server and back -- i.e. one *ping pong* message. To control the number of messages sent and the size of each message, each master executable (which stars the server and client) takes two optional command-line arguments:

* `-c <count>`: How many messages to send between the server and client. Defaults to 1000.
* `-s <size>`: The size of individual messages. Defaults to 1000.

For example, you can measure the latency of sending 100 bytes a million times via domain sockets with the following command:

```shell
$ ./domain -c 1000000 -s 100
```

We also provide a shell script under `results/` that runs all methods with various configurations and stores the results. Some tests may have issues due to system limits, so you may want to re-run the script or run some tests manually.

## [License](http://goldsborough.mit-license.org)

This project is released under the [MIT License](http://goldsborough.mit-license.org). For more information, see the LICENSE file.

## Authors

[Peter Goldsborough](http://goldsborough.me) + [cat](https://goo.gl/IpUmJn) :heart:

<a href="https://gratipay.com/~goldsborough/"><img src="http://img.shields.io/gratipay/goldsborough.png?style=flat-square"></a>
