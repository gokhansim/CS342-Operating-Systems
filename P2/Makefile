
all: cfs loadgen

cfs: cfs.c
	g++ cfs.c -o cfs

loadgen: loadgen.c
	g++ loadgen.c -o loadgen

clean:
	rm -fr *~ *.o cfs loadgen
