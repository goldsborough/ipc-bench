# IPC-Bench

[![GitHub license](https://img.shields.io/github/license/mashape/apistatus.svg?style=flat-square)](http://goldsborough.mit-license.org)

Implementations and benchmarks for various inter-process-communication (IPC) methods on Linux and OS X.

## Spectrum

The following IPC methods are implemented.
To measure their sequential throughput we send a single message forth _and_ back (i.e., *ping pong*) between two processes.

| Method                  |         100 Byte Messages |       1 Kilo Byte Messages |
| ----------------------- | -------------------------:| --------------------------:|
| Unix Signals            |                --broken-- |                 --broken-- |
| ZeroMQ (TCP)            |              24,901 msg/s |               22,679 msg/s |
| Internet sockets (TCP)  |              70,221 msg/s |               67,901 msg/s |
| Domain sockets          |             130,372 msg/s |              127,582 msg/s |
| Pipes                   |             162,441 msg/s |              155,404 msg/s |
| Message Queues          |             232,253 msg/s |              213,796 msg/s |
| FIFOs (named pipes)     |             265,823 msg/s |              254,880 msg/s |
| Shared Memory           |           4,702,557 msg/s |            1,659,291 msg/s |
| Memory-Mapped Files     |           5,338,860 msg/s |            1,701,759 msg/s |

###### Benchmarked on ``Intel(R) Core(TM) i5-4590S CPU @ 3.00GHz`` running ``Ubuntu 20.04.1 LTS``.

**NOTE**: The code is rather old and there might be sub-optimal configurations!
We are happy to update the configuration with concrete suggestions (see contributions below).
In particular, ``zeromq`` is a great library and should probably be performing better, especially because we are only using the TCP implementation.
There is one dedicated for IPC (-> TODO).
In addition, there is little technical reason for shared memory to perform differently than memory-mapped files (could be due to a lack of warmup).
Non-the-less, hopefully, this benchmark can serve as a solid starting point by providing ball-park numbers and a reference implementation.

For a detailed evaluation of inter-node communication, see our [L5 library](https://github.com/pfent/L5RDMA).

## Usage

Some required packages on Ubuntu:
```shell
sudo apt-get install pkg-config, libzmqpp-dev
```

You can build the project and all necessary executables using CMake. The following commands (executed from the root folder) should do the trick:

```shell
mkdir build
cd build
cmake ..
make
```

This will generate a `build/source` folder, holding further directories for each IPC type.
Simply execute the program named after the folder, e.g. `build/source/shm/shm`.
Where applicable, this will start a new server and client process, run benchmarks and print results to `stdout`. For example, running `build/source/shm/shm` outputs:

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

To control the number of messages sent and the size of each message, each master executable (which stars the server and client) takes two optional command-line arguments:

* `-c <count>`: How many messages to send between the server and client. Defaults to 1000.
* `-s <size>`: The size of individual messages. Defaults to 1000.

For example, you can measure the latency of sending 100 bytes a million times via domain sockets with the following command:

```shell
$ ./domain -c 1000000 -s 100
```

We also provide a shell script under `results/` that runs all methods with various configurations and stores the results.
Some tests may have issues due to system limits, so you may want to re-run the script or run some tests manually.

## Contributions

Contributions are welcome, as long as they fit within the goal of this benchmark: sequential single-node communication.

Just open an issues or send a pull request.

## [License](http://goldsborough.mit-license.org)

This project is released under the [MIT License](http://goldsborough.mit-license.org). For more information, see the LICENSE file.

## Authors

[Peter Goldsborough](http://goldsborough.me) + [cat](https://goo.gl/IpUmJn) :heart:

Some maintenance by [Alexander van Renen](https://github.com/alexandervanrenen)
