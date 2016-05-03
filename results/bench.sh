#!/bin/bash

root=../build

for tech in shm domain fifo pipe mmap tcp zeromq mq; do
		echo "Running $tech ...."

		if [ -f output/$tech.result ]; then
				rm output/$tech.result
		fi

		for size_power in $(seq 0 3); do
				size=$((2**size_power))
				for count_power in $(seq 0 3); do
						count=$((10**count_power))
						$root/$tech/$tech -s $size -c $count >> output/$tech.result
				done
		done

		# Clean up, just in case
		killall "$tech-server" &> /dev/null
		killall "$tech-client" &> /dev/null
done
