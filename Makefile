# Usage:
# make			# compile C programs to binary
# run			# run the output file
# make clean	# remove compiled binaries and output files


makelist:
	gcc makelist.c -o makelist

run:
	./makelist 10000 32

clean:
	rm makelist
	rm numbers.txt