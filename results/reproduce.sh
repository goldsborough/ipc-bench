#!/bin/bash -i

./build/source/domain/domain -c 500000 -s 100
./build/source/fifo/fifo -c 500000 -s 100
./build/source/mmap/mmap -c 5000000 -s 100
./build/source/mq/mq -c 200000 -s 100
./build/source/pipe/pipe -c 200000 -s 100
./build/source/shm/shm -c 5000000 -s 100
./build/source/tcp/tcp -c 100000 -s 100
./build/source/zeromq/zeromq -c 20000 -s 100

./build/source/domain/domain -c 500000 -s 1000
./build/source/fifo/fifo -c 500000 -s 1000
./build/source/mmap/mmap -c 5000000 -s 1000
./build/source/mq/mq -c 200000 -s 1000
./build/source/pipe/pipe -c 200000 -s 1000
./build/source/shm/shm -c 5000000 -s 1000
./build/source/tcp/tcp -c 100000 -s 1000
./build/source/zeromq/zeromq -c 20000 -s 1000
