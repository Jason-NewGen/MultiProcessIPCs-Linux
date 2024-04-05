# Usage:
# make			# compile C programs to binary
# make run			# run the output file
# make clean	# remove compiled binaries and output files


make:
	gcc makelist.c -o makelist
	gcc getAvgMaxH.c -o getAvgMaxH

run:
	./makelist 10000 32
	./getAvgMaxH 5

clean:
	rm -f makelist
	rm -f getAvgMaxH
	rm -f numbers.txt
	rm -f outputbfs.txt
	rm -f outputdfs.txt