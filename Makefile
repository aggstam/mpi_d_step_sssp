DSTEP = 0.25
FILE = grph_1000
OUTPUT = output
PROCESSES = 4

all:
	$(info Executing normal code...)
	gcc -o d_step_sssp d_step_sssp.c
	./d_step_sssp $(DSTEP) $(FILE) $(OUTPUT)

mpi:
	$(info Executing MPI code...)
	mpicc -lm -o mpi_d_step_sssp mpi_d_step_sssp.c
	mpiexec -np $(PROCESSES) ./mpi_d_step_sssp $(DSTEP) $(FILE) $(OUTPUT)

clean:
	rm -f d_step_sssp mpi_d_step_sssp output
