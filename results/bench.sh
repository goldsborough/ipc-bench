#!/bin/bash -i

result_directory=$(pwd)
output="$result_directory/output"

if [ ! -d $output ]; then
		mkdir $output
fi

cd ${result_directory%/ipc-bench*}/ipc-bench/build/source

technologies=(
		shm
		mq
		domain
		fifo
		pipe
		mmap
		tcp
		signal
		zeromq
)

if [ $(uname) = Linux ]; then
		technologies+=( eventfd-uni )
fi

for tech in $technologies; do
		echo "Running $tech ..."
		cd $tech

		if [ -f output/$tech.result ]; then
				rm $output/$tech.result
		fi

		for size_power in $(seq 0 3); do
				size=$((10**size_power))
				for count_power in $(seq 0 3); do
						count=$((10**count_power))
						./$tech -s $size -c $count >> "$output/$tech.result"
						if [ $tech = zeromq ]; then
								sleep 0.2
						else
								sleep 0.1
						fi
				done
		done

		# Clean up, just in case
		killall -9 "$tech-server" &> /dev/null
		killall -9 "$tech-client" &> /dev/null

		cd ..
done

cd $result_directory
