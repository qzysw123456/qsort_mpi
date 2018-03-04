EXECSS=qs_mpi
qsort: qs_mpi.c
	mpicc -o qs_mpi qs_mpi.c

clean:
	rm -f qs_mpi
